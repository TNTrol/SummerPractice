#include "utils.h"

int create_socket(char url_str[], BIO *out)
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
        BIO_printf(out, "Error: Cannot resolve hostname %s.\n",  hostname);
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
        BIO_printf(out, "Error: Cannot connect to host %s [%s] on port %d.\n",
                   hostname, tmp_ptr, port);
    }

    return sockfd;
}

 int scan_server(char *dest_url)
{
    BIO              *certbio = NULL;
    BIO               *outbio = NULL;
    X509                *cert = NULL;
    X509_NAME       *certname = NULL;
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    SSL *ssl;
    int server = 0;
    int ret, i;

    /* ---------------------------------------------------------- *
     * These function calls initialize openssl for correct work.  *
     * ---------------------------------------------------------- */
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    //certbio = BIO_new(BIO_s_file());
    outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);
    BIO_printf(outbio, "Could not initialize the OpenSSL library !\n");

    //метод для TLS
    method = SSLv23_client_method();
    if ( (ctx = SSL_CTX_new(method)) == NULL)
        BIO_printf(outbio, "Unable to create a new SSL context structure.\n");

    //SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    ssl = SSL_new(ctx);
    server = create_socket(dest_url, outbio);
    if(server != 0)
        BIO_printf(outbio, "Successfully made the TCP connection to: %s.\n", dest_url);

    SSL_set_fd(ssl, server);

    if ( SSL_connect(ssl) != 1 )
        BIO_printf(outbio, "Error: Could not build a SSL session to: %s.\n", dest_url);
    else
        BIO_printf(outbio, "Successfully enabled SSL/TLS session to: %s.\n", dest_url);

    //получение сертификата
    cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL)
        BIO_printf(outbio, "Error: Could not get a certificate from: %s.\n", dest_url);
    else
        BIO_printf(outbio, "Retrieved the server's certificate from: %s.\n", dest_url);

    certname = X509_get_subject_name(cert);
    BIO_printf(outbio, "Displaying the certificate subject data:\n");
    X509_NAME_print_ex(outbio, certname, 0, 0);
    BIO_printf(outbio, "\n");

    EVP_PKEY *public_key = X509_get_pubkey(cert);


    SSL_CIPHER *chipher = SSL_get_current_cipher(ssl);
    char * a = SSL_CIPHER_get_name(chipher);
    printf("Chipher: %s\n", a);
    printf("Version: %s\n", SSL_CIPHER_get_version(chipher));

    int id = EVP_PKEY_base_id(public_key);
    int key = -1;
    printf("%s\n",SSL_get_version(ssl));
    if(EVP_PKEY_RSA == id)
    {
        RSA *rsa_key = EVP_PKEY_get0_RSA(public_key);
        key = RSA_size(rsa_key);
    }
    else if(EVP_PKEY_DH == id)
    {
        DH *dh_key = EVP_PKEY_get0_DH(public_key);
        key = DH_bits(dh_key);
    }
    else if(EVP_PKEY_EC == id)
    {
        EC_KEY *ec = EVP_PKEY_get0_EC_KEY(public_key);
        key = EC_GROUP_order_bits(EC_KEY_get0_group(ec));
    }
    else if(EVP_PKEY_DSA == id)
    {
        DSA *dsa = EVP_PKEY_get0_DSA(public_key);
        key = DSA_bits(dsa);
    }

    if(key > -1)
        printf("Length of key: %d\n", key);

    //освобождение памяти
    SSL_free(ssl);
    close(server);
    X509_free(cert);
    SSL_CTX_free(ctx);
    BIO_printf(outbio, "Finished SSL/TLS connection with server: %s.\n", dest_url);
    return(0);
}