

#ifdef __cplusplus
extern "C" {                 // Make sure we have C-declarations in C++ programs
#endif

#ifndef SCANNER_H
#define SCANNER_H
//#define DEBUG
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <unistd.h>
#include "utils.h"
#define COUNT_VERSION 4
#define PORT 443
#define PORT_CHARS "443"

static const int Versions[COUNT_VERSION] = {TLS1_VERSION, TLS1_1_VERSION, TLS1_2_VERSION, TLS1_3_VERSION};
int create_socket(char url_str[], char **out);
void convert_cipher_ssl_to_cipher(CipherSSL*cipher_ssl, TlsInformation*tls);
Report * scan_server(char url_str[]);
Report * scan_server_with_error(char url_str[], char **err);

#if defined(__cplusplus)
}
#endif

#endif