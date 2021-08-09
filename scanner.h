

#ifdef __cplusplus
extern "C" {                 // Make sure we have C-declarations in C++ programs
#endif

#ifndef SCANNER_H
#define SCANNER_H
#define DEBUG
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

int create_socket(char url_str[], BIO *out);
int scan_server(char url_str[]);
int scan_server_2(char url_str[]);

#if defined(__cplusplus)
}
#endif

#endif