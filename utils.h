#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>

struct cipher{
    const char *version;
    const char *name;
    int length;
};

typedef struct cipher Cipher;

struct  options{
    char *file_o, *file_f;
    char **servers;
    int size;
};

struct report{
    const char *target;
    Cipher tls_max_version;
    Cipher tls_min_version;
};

enum Version{
    MIN = 0,
    MAX = 1
};

enum ResultScanning{
    NOT_FOUND_SERVER = 0,
    DONT_SUPPORT_VERSION_SSL = -1,
    SUCCESS = 1,
    ERROR = -2
};

typedef struct report Report;
typedef struct options Options;

static inline Report * create_report()
{
    Report *report= malloc (sizeof (Report));
    report->target = NULL;
    report->tls_min_version.length = 0;
    report->tls_min_version.name = NULL;
    report->tls_min_version.version = NULL;
    report->tls_max_version.length = 0;
    report->tls_max_version.name = NULL;
    report->tls_max_version.version = NULL;
    return report;
}

//static inline void free_options(struct options *options)
//{
//    if(options->file_o)
//        free(options->file_o);
//    if(options->file_f)
//        free(options->file_f);
//    if(options->servers)
//        free(options->servers);
//}

#endif