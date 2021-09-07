#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "thread_pool.h"
//структуры для хранения всех данных сканирования
struct cipher_information
{
    const char *name;
    int length_of_key;
};

struct tls_information{
    const char *version;
    struct cipher_information *ciphers;
    int count_ciphers;
};

struct cipher_ssl;
typedef struct tls_information TlsInformation;

struct  options{
    char *file_o;
    char *file_f;
    char **servers;
    int size;
};

struct report{
    const char *target;
    TlsInformation tls_max_version;
    TlsInformation tls_min_version;
};

//для методов сканирования с помощью потоков
struct ctx_of_thread{
    pthread_mutex_t mutex;
    struct report **reports;
    int size;
    int count;
};

struct thread_arg{
    char *url;
    struct ctx_of_thread *thread_data;
};

//это для методов сканирования с помощью пула потоков
struct thread_all_data
{
    struct ctx_of_thread *ctx;
    char **url;
};


//резульат сканирования, почему не #difine просто не знал как правильно
enum ResultScanning{
    NOT_FOUND_SERVER = 0,
    DONT_SUPPORT_VERSION_SSL = -1,
    SUCCESS = 1,
    ERROR = -2
};

typedef struct cipher_information Cipher;
typedef struct report Report;
typedef struct options Options;
typedef struct thread_arg ThreadArg;
typedef struct ctx_of_thread ThreadConst;
typedef struct cipher_ssl CipherSSL;
typedef struct thread_all_data ThreadAllData;

static inline void default_cipher(TlsInformation *tls)
{
    tls->version = NULL;
    tls->ciphers = NULL;
    tls->count_ciphers = 0;
}

static inline Report * create_report()
{
    Report *report= malloc (sizeof (Report));
    report->target = NULL;
    default_cipher(&report->tls_max_version);
    default_cipher(&report->tls_min_version);
    return report;
}

static inline void free_report(Report* report)
{
    if(report == NULL)
        return;
    free(report->tls_min_version.ciphers);
    free(report->tls_max_version.ciphers);
}


#endif