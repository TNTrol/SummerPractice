#ifndef UTILS_H
#define UTILS_H

struct  options{
    char *file_o, *file_f;
    char **servers;
    int size;
};

struct report{
    int length;
    char *target;
    char *min_cipher, *max_cipher;
    char *min_version, *max_version;
};

typedef struct report Report;
typedef struct options Options;
//
//void free_options(struct options *options);
//
//inline void free_options(struct options *options)
//{
//    if(options->file_o)
//        free(options->file_o);
//    if(options->file_f)
//        free(options->file_f);
//    if(options->servers)
//        free(options->servers);
//}

#endif