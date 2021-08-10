

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
#include "utils.h"

static const int versions[3] = {TLS1_1_VERSION, TLS1_2_VERSION, TLS1_3_VERSION};
int create_socket(char url_str[], char **out);
int scan_server(char url_str[], void (*print) (Report *));

#if defined(__cplusplus)
}
#endif

#endif