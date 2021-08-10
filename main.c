//#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scanner.h"
#include "utils.h"

void print(Report *report)
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
    Options op = {.file_f = NULL, .size = 0, .file_o = 0, .servers = NULL};
    int result = 0;
    char **out;
    int size = 0;
    Report **reports;

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
            Report *report = scan_server(line);
            if(report)
            {
                print(report);
                free(report);
            }
        }
        free(line);
        fclose(fp);
    }else{
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
    }

    if(op.file_o)
    {
        FILE *fp;
        size_t count;
        fp = freopen(op.file_o, "wb", stdout);
        if(fp == NULL) {
            puts("Error in open file");
            goto free_;
        }
        for(int i = 0; i < size; i++)
        {
            print(reports[i]);
        }
        fclose(fp);
    }else if (! op.file_f)
    {
        for(int i = 0; i < size; i++)
        {
            print(reports[i]);
        }
    }

    free_:
    if(reports) {
        for (int i = 0; i < size; i++)
            if (reports[i])
                free(reports[i]);
        free(reports);
    }
    return result;
}
