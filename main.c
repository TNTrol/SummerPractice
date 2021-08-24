//#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scanner.h"
#include "utils.h"
#include "visiting_servers.h"
#define SIZE 10

void print_name_cipher(TlsInformation *cipher, char* version)
{
    printf("\n%s version: %s \nSupported ciphers:\n", version, cipher->version);
    for(int i = 0; i < cipher->count_ciphers; ++i)
    {
        printf("Name: %s\t Length: %d\n", cipher->ciphers[i].name, cipher->ciphers[i].length_of_key);
    }
}

void printReport(Report *report)
{
    puts("___________Report__________");
    printf("Target: %s\n",report->target);
    print_name_cipher(&report->tls_min_version, "Minimal");
    print_name_cipher(&report->tls_max_version, "Maximal");
}

void printReports(Report **reports, int size)
{
    for(int i = 0; i < size; i++)
    {
        printReport(reports[i]);
    }
}

int read_name_of_file(int argc, char **argv, int *index, char **out_name)
{
    (*index)++;
    int i = *index;
    if(*out_name)
    {
        puts("This argument already exists");
        return 1;
    }
    if(argc <= i)
    {
        puts("File ciphers expected");
        return 1;
    }
    *out_name = argv[i];
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        puts("Invalid number of parameters");
        return 1;
    }
    Options op = {.file_f = NULL, .size = 0, .file_o = NULL, .servers = NULL};
    int result = 0;
    int size = 0;
    int max_size = SIZE;
    Report **reports = NULL;
    FILE *fp = NULL;

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-f") == 0)
        {
            if(read_name_of_file(argc, argv, &i, &op.file_f))
            {
                result = -1;
                goto free_;
            }
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            if(read_name_of_file(argc, argv, &i, &op.file_o))
            {
                result = -1;
                goto free_;
            }
        }
        else
        {
            if(!op.servers)
                op.servers = argv + i;
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
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen(op.file_f, "r");
        if (fp == NULL)
        {
            printf("Error in open file %s", op.file_f);
            result = -1;
            goto free_;
        }
        op.servers = malloc(sizeof(char *) * max_size);
        while ((read = getline(&line, &len, fp)) != -1)
        {
            if (read <= 2)
                continue;
            if (line[read - 1] == '\n')
                line[read - 1] = 0;
            if (max_size <= op.size)
            {
                max_size *= 2;
                op.servers = realloc(op.servers, max_size * sizeof(char *));
            }
            op.servers[op.size] = line;
            op.size++;
            line = NULL;
        }
        fclose(fp);
    }

    reports = malloc(op.size * sizeof(Report *));
    //size = threading_visit(op.size, op.servers, reports);//
    //size = serial_visit(op.size, op.servers, reports); //
    size = threading_visit_with_thread_pool(op.size, op.servers, reports);


    if(op.file_o)
    {
        fp = freopen(op.file_o, "wb", stdout);
        if(fp == NULL)
        {
            printf("Error in open file %s", op.file_o);
            result = -1;
            goto free_;
        }
        printReports(reports, size);
        fclose(fp);
    }
    else
    {
        printReports(reports, size);
    }


    free_:
    if(op.file_f)
    {
        for(int i = 0; i < op.size; i++)
            free(op.servers[i]);
        free(op.servers);
    }
    if(reports)
    {
        for (int i = 0; i < size; i++)
                free_report(reports[i]);
        free(reports);
    }
    return result;
}
