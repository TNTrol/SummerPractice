#include "scanner.h"

int create_socket(char url_str[], char **err)
{
    int sockfd;
    char hostname[256] = "";
    char    portnum[6] = "443";
    char      proto[6] = "";
    char      *tmp_ptr = NULL;
    int           port;
    struct hostent *host;
    struct sockaddr_in dest_addr;
    if(url_str[strlen(url_str)] == '/')
        url_str[strlen(url_str)] = '\0';
    strncpy(proto, url_str, (strchr(url_str, ':')-url_str));
    strncpy(hostname, strstr(url_str, "://")+3, sizeof(hostname));
    if(strchr(hostname, ':')) {
        tmp_ptr = strchr(hostname, ':');
        /* the last : starts the port number, if avail, i.e. 8443 */
        strncpy(portnum, tmp_ptr+1,  sizeof(portnum));
        *tmp_ptr = '\0';
    }

    port = atoi(portnum);

    if ( (host = gethostbyname(hostname)) == NULL ) {
//        BIO_printf(out, "Error: Cannot resolve hostname %s.\n",  hostname);
        *err = "Cannot resolve hostname";
        return 0;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    dest_addr.sin_family=AF_INET;
    dest_addr.sin_port=htons(port);
    dest_addr.sin_addr.s_addr = *(long*)(host->h_addr);
    memset(&(dest_addr.sin_zero), '\0', 8);

    tmp_ptr = inet_ntoa(dest_addr.sin_addr);
    if ( connect(sockfd, (struct sockaddr *) &dest_addr,
                 sizeof(struct sockaddr)) == -1 ) {
        *err = "Cannot connect to host";
        return 0;
    }

    return sockfd;
}


int scan_server_version(char *dest_url, int version, Cipher *cipher, char **err_msg)
{
    X509 *cert = NULL;
    const SSL_METHOD *method = NULL;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int server = 0, key = -1, id;
    enum ResultScanning result = SUCCESS;
    method = TLS_client_method();
    if ( (ctx = SSL_CTX_new(method)) == NULL) {
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
    if (SSL_connect(ssl) != 1) {
        *err_msg = "Could not build a SSL session to";
        result = DONT_SUPPORT_VERSION_SSL;
        goto mem_free;
    }
    //получение сертификата
    cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL) {
        *err_msg = "Could not get a certificate";
        result = ERROR;
        goto mem_free;
    }
    EVP_PKEY *public_key = X509_get_pubkey(cert);
    const SSL_CIPHER *chipher = SSL_get_current_cipher(ssl);
    cipher->version = SSL_get_version(ssl);
    cipher->name = SSL_CIPHER_get_name(chipher);

    id = EVP_PKEY_base_id(public_key);
    if (EVP_PKEY_RSA == id) {
        RSA *rsa_key = EVP_PKEY_get0_RSA(public_key);
        key = RSA_size(rsa_key);
    } else if (EVP_PKEY_DH == id) {
        DH *dh_key = EVP_PKEY_get0_DH(public_key);
        key = DH_bits(dh_key);
    } else if (EVP_PKEY_EC == id) {
        EC_KEY *ec = EVP_PKEY_get0_EC_KEY(public_key);
        key = EC_GROUP_order_bits(EC_KEY_get0_group(ec));
    } else if (EVP_PKEY_DSA == id) {
        DSA *dsa = EVP_PKEY_get0_DSA(public_key);
        key = DSA_bits(dsa);
    }
    if (key > -1){
        cipher->length = key;
#ifdef DEBUG
        printf("L = %d\n", key);
#endif
    }

    mem_free:
    if(ssl)
        SSL_free(ssl);
    if(server > 0)
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
    Cipher *cipher = &(report->tls_min_version);
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    if(SSL_library_init() < 0) {
        *err_msg = "Could not initialize the OpenSSL library !";
        return -1;
    }
    for(int i = 0; i < 3; i++)
    {
        res = scan_server_version(dest_url, Versions[i], cipher, err_msg);
        switch (res) {
            case SUCCESS:{
                if(type == 0)
                {
                    cipher = &(report->tls_max_version);
                    cipher->version == report->tls_min_version.version;
                    cipher->name = report->tls_min_version.name;
                    cipher->length = report->tls_min_version.length;
                    type++;
                }
                break;
            }
            case NOT_FOUND_SERVER:
                return -1;
            case DONT_SUPPORT_VERSION_SSL:
                break;
            default:
                break;
        }
    }
    if(type > 0){
        report->target = dest_url;
        return 1;
    }
    return -1;
}


Report *scan_server(char *url_str)
{
    Report *report  = create_report();
    char *err = NULL;
    if(scan_server_report(url_str, report, &err) < 0){
        printf( "Target: %s.\n Error: %s", url_str, err);
        free(err); //падает
        free(report);
        return NULL;
    }
    return report;
}