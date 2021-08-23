#include "scanner.h"
#define DEBUG

struct cipher_ssl{
    struct stack_st_SSL_CIPHER *ciphers_stack;
    const char *version;
};

CipherSSL *create_cipher_SSL()
{
    CipherSSL *cipher = malloc(sizeof (CipherSSL));
    cipher->version = NULL;
    cipher->ciphers_stack = NULL;
    return cipher;
}

int create_socket(char url_str[], char **err)
{
    int sock_fd;
    char hostname[256] = "";
    char    port_num[6] = PORT_CHARS;
    char      *tmp_ptr = NULL, *addr_ptr = NULL;
    int           port = PORT;
    struct hostent *host;
    struct sockaddr_in dest_addr;
    if(url_str[strlen(url_str)] == '/')
    {
        url_str[strlen(url_str)] = '\0';
    }
    addr_ptr = strstr(url_str, "://");
    if(addr_ptr)
    {
        strncpy(hostname, addr_ptr + 3, sizeof(hostname));
    }
    else
    {
        strncpy(hostname, url_str, sizeof(hostname));
    }
    if(strchr(hostname, ':')) {
        tmp_ptr = strchr(hostname, ':');
        strncpy(port_num, tmp_ptr + 1, sizeof(port_num));
        *tmp_ptr = '\0';
    }

    if ( (host = gethostbyname(hostname)) == NULL ) {
        *err = "Cannot resolve hostname";
        return 0;
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    dest_addr.sin_family=AF_INET;
    dest_addr.sin_port=htons(port);
    dest_addr.sin_addr.s_addr = *(long*)(host->h_addr);
    memset(&(dest_addr.sin_zero), '\0', 8);

    if (connect(sock_fd, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr)) == -1 )
    {
        *err = "Cannot connect to host";
        return 0;
    }

    return sock_fd;
}


int scan_server_cipher(char *dest_url, int version, CipherSSL *cipher, char **err_msg)
{
    X509 *cert = NULL;
    const SSL_METHOD *method = NULL;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int server = 0;
    enum ResultScanning result = SUCCESS;
    method = TLS_client_method();
    if ( (ctx = SSL_CTX_new(method)) == NULL)
    {
        *err_msg = "Unable to create a new SSL context structure";
        result = ERROR;
        goto mem_free;
    }
    server = create_socket(dest_url, err_msg);
    if(server == 0)
    {
        *err_msg = "Unable to create the TCP connection";
        result = NOT_FOUND_SERVER;
        goto mem_free;
    }
    ssl = SSL_new(ctx);
    SSL_set_max_proto_version(ssl, version);
    SSL_set_min_proto_version(ssl, version);
    SSL_set_fd(ssl, server);
    if (SSL_connect(ssl) != 1)
    {
        *err_msg = "Could not build a SSL session";
        result = DONT_SUPPORT_VERSION_SSL;
        goto mem_free;
    }
    //получение сертификата
    cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL)
    {
        *err_msg = "Could not get a certificate";
        result = ERROR;
        goto mem_free;
    }

    cipher->version = SSL_get_version(ssl);
    sk_SSL_CIPHER_free(cipher->ciphers_stack);
    cipher->ciphers_stack = SSL_get1_supported_ciphers(ssl);
#ifdef DEBUG1
        printf("L = %d\n", key);
#endif


    mem_free:
    if(ssl)
        SSL_free(ssl);
    if(server != 0)
        close(server);
    if(cert)
        X509_free(cert);
    if(ctx)
        SSL_CTX_free(ctx);
    return result;
}

int scan_server_report(char *dest_url, Report *report, char **err_msg)
{
    int res = 1, type = 0;
    CipherSSL *min_cipher = NULL, *curr_cipher = NULL;
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    if(SSL_library_init() < 0)
    {
        *err_msg = "Could not initialize the OpenSSL library !";
        return -1;
    }

    curr_cipher = create_cipher_SSL();
    for(int i = 0; i < COUNT_VERSION; i++)
    {
        *err_msg = NULL;
        res = scan_server_cipher(dest_url, Versions[i], curr_cipher, err_msg);
#ifdef DEBUG
        printf("RES = %d of %s\n", res, dest_url);
#endif
        switch (res)
        {
            case SUCCESS:
            {
                if(type == 0)
                {
                    min_cipher = curr_cipher;
                    curr_cipher = NULL;
                    curr_cipher = create_cipher_SSL();
                }
                type++;
                break;
            }
            case NOT_FOUND_SERVER:
            {
                res = -1;
                goto mem_free;
            }
            case DONT_SUPPORT_VERSION_SSL:
                break;
            default:
                break;
        }
    }
    if(type > 0)
    {
        report->target = dest_url;
        convert_cipher_ssl_to_cipher(min_cipher, &report->tls_min_version);
        if(type == 1)
        {
            report->tls_max_version = report->tls_min_version;
        }
        else
        {
            convert_cipher_ssl_to_cipher(curr_cipher, &report->tls_max_version);
        }
        res = 1;
    }
    mem_free:
    if(min_cipher)
        sk_SSL_CIPHER_free(min_cipher->ciphers_stack);
    free(min_cipher);
    sk_SSL_CIPHER_free(curr_cipher->ciphers_stack);
    free(curr_cipher);
    return res;
}


Report *scan_server(char *url_str)
{
    char *err = NULL;
    Report *report  = scan_server_with_error(url_str, &err);
    if(!report)
    {
        printf( "Target: %s.\nError: %s\n", url_str, err);
        free(report);
        return NULL;
    }
    return report;
}

Report * scan_server_with_error(char url_str[], char **err)
{
    Report *report  = create_report();
    if(scan_server_report(url_str, report, err) < 0)
    {
        free(report);
        return NULL;
    }
    return report;
}

void convert_cipher_ssl_to_cipher(CipherSSL *cipher_ssl, TlsInformation *tls)
{
    const SSL_CIPHER *cipher_temp = NULL;
    tls->version = cipher_ssl->version;
    tls->count_ciphers = sk_SSL_CIPHER_num(cipher_ssl->ciphers_stack);
    tls->ciphers = malloc(sizeof(Cipher) * tls->count_ciphers);
    for(int i = 0; i < tls->count_ciphers; ++i)
    {
        cipher_temp = sk_SSL_CIPHER_pop(cipher_ssl->ciphers_stack);
        tls->ciphers[i].name = SSL_CIPHER_get_name(cipher_temp);
        tls->ciphers[i].length_of_key = SSL_CIPHER_get_bits(cipher_temp, NULL);
    }
}
