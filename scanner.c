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
        abort();
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    dest_addr.sin_family=AF_INET;
    dest_addr.sin_port=htons(port);
    dest_addr.sin_addr.s_addr = *(long*)(host->h_addr);
    memset(&(dest_addr.sin_zero), '\0', 8);

    tmp_ptr = inet_ntoa(dest_addr.sin_addr);
    if ( connect(sockfd, (struct sockaddr *) &dest_addr,
                 sizeof(struct sockaddr)) == -1 ) {
//        BIO_printf(out, "Error: Cannot connect to host %s [%s] on port %d.\n",
//                   hostname, tmp_ptr, port);
    *err = "Cannot connect to host";
    }

    return sockfd;
}


int scan_server_version(char *dest_url, Report *report, char **err_msg, enum Version lim)
{
    static const int versions[2] = {TLS1_1_VERSION, TLS_MAX_VERSION};
    X509 *cert = NULL;
    const SSL_METHOD *method;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int server = 0, result = 1, key = -1, id;
    method = TLS_client_method();
    if ( (ctx = SSL_CTX_new(method)) == NULL) {
        *err_msg = "Unable to create a new SSL context structure";
        result = -1;
        goto mem_free;
    }
    server = create_socket(dest_url, err_msg);
    if(server == 0)
    {
        *err_msg = "Unable to create the TCP connection";
        result = -1;
        goto mem_free;
    }
    ssl = SSL_new(ctx);
    SSL_set_max_proto_version(ssl, versions[lim]);
    //SSL_set_min_proto_version(ssl, versions[lim]);
    SSL_set_fd(ssl, server);
    if (SSL_connect(ssl) != 1) {
        *err_msg = "Could not build a SSL session to";
        result = -1;
        goto mem_free;
    }
    //получение сертификата
    cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL) {
        *err_msg = "Could not get a certificate";
        result = -1;
        goto mem_free;
    }
    EVP_PKEY *public_key = X509_get_pubkey(cert);
    const SSL_CIPHER *chipher = SSL_get_current_cipher(ssl);
    Cipher cipher = {.version = SSL_get_version(ssl), .name = SSL_CIPHER_get_name(chipher)};
    report->ciphers[lim] = cipher;

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
    if (key > -1)
        report->length = key;
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
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    if(SSL_library_init() < 0) {
        *err_msg = "Could not initialize the OpenSSL library !";
        return -1;
    }
    if( scan_server_version(dest_url, report, err_msg, MIN) > 0 && scan_server_version(dest_url, report, err_msg, MAX) > 0)
    {
        report->target = dest_url;
        return 1;
    }
    return -1;
}

int scan_server(char *url_str, void (*print) (Report *))
{

    Report report = {.min_version = NULL, .min_cipher = NULL, .length = 0, };
    char *err = NULL;
    if(scan_server_report(url_str, &report, &err) < 0){
        printf( "Target: %s.\n Error: %s", url_str, err);
        //free(err);
        return -1;
    }
    print(&report);
    return 0;
}