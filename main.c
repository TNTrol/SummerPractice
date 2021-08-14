//#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scanner.h"
#include "utils.h"
#define SIZE 10

void printReport(Report *report)
{
    puts("___________Report__________");
    printf("Target: %s\n"
           "Length: %d\n"
           "Minimal version:"
           "\n Cipher: %s, Version: %s\n"
           "Maximal version:\n "
           "Cipher: %s, Version: %s\n",
           report->target,
           report->tls_min_version.length,
           report->tls_min_version.name, report->tls_min_version.version,
           report->tls_max_version.name, report->tls_max_version.version );
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
        puts("File name expected");
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
                op.servers = argv + i;
            op.size++;
        }
        if(op.size > 0 && op.file_f)
        {
            puts("Data entry is not possible from file and console at the same time");
            goto free_;
        }
    }

    if(op.file_f) {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen(op.file_f, "r");
        if (fp == NULL) {
            printf("Error in open file %s", op.file_f);
            goto free_;
        }
        op.servers = malloc(sizeof(char *) * max_size);
        while ((read = getline(&line, &len, fp)) != -1) {
            if (read <= 2)
                continue;
            if (line[read - 1] == '\n')
                line[read - 1] = 0;
            if (max_size <= op.size) {
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
    puts("Start scanning...");
    for (int i = 0; i < op.size; i++ )
    {
        Report *report = scan_server(*(op.servers + i));
        if(report)
        {
            reports[size] = report;
            size++;
        }
        printf("%d of %d\n", i + 1, op.size);
    }

    if(op.file_o)
    {
        fp = freopen(op.file_o, "wb", stdout);
        if(fp == NULL) {
            printf("Error in open file %s", op.file_o);
            goto free_;
        }
        printReports(reports, size);
        fclose(fp);
    }else
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
    if(reports) {
        for (int i = 0; i < size; i++)
                free(reports[i]);
        free(reports);
    }
    return result;
}
