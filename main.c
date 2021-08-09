//#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scanner.h"
#include "utils.h"
#include <unistd.h>

int read_name_of_file(int argc, char **argv, int *index, char **out_name)
{
    (*index)++;
    int i = *index;
    if(*out_name)
    {
        puts("Такой аргумент уже есть");
        return 1;
    }
    if(argc <= i)
    {
        puts("Ожидалось название файла");
        return 1;
    }
    *out_name = argv[i];
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("Invalid number of parameters");
        return 1;
    }
    Options op = {.file_f = NULL, .size = 0, .file_o = 0, .servers = NULL};
    int result = 0;
    char **out;

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-f") == 0)
        {
            puts("ok2");
            if(read_name_of_file(argc, argv, &i, &op.file_f)){
                result = -1;
                goto free_;
            }
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            if(read_name_of_file(argc, argv, &i, &op.file_o)) {
                result = -1;
                goto free_;
            }
        }
        else {
            if(!op.servers)
                op.servers = argv + 1;
            op.size++;
        }
        if(op.size > 0 && op.file_f)
        {
            puts("Data entry is not possible from file and console at the same time");
            goto free_;
        }
    }

    if(op.file_f)
    {
        FILE *fp = NULL;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen(op.file_f, "r");
        if (fp == NULL)
        {
            puts("Error in open file");
            goto free_;
        }
        while ((read = getline(&line, &len, fp)) != -1) {
            if(read <= 2)
                continue;
            if(line[read - 1] == '\n')
                line[read - 1] = 0;
            scan_server(line);
        }
        free(line);
    }
    else
    {
        for (int i = 0; i < op.size; i++ )
            scan_server_2( *(op.servers + i) );
    }

    if(op.file_o)
    {
        puts("The function of writing in file is not implemented");
    }
    free_:
    return result;
}