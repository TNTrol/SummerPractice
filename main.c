//#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scanner.h"
#include "utils.h"
#include <pthread.h>
#define SIZE 10

static int counter = 0, size_s = 0, curr_i = 0;
pthread_mutex_t mutex;
static Report **reports_s;

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

void finish_scanning(Report *report)
{
    pthread_mutex_lock(&mutex);
    printf("%d of %d\n", curr_i + 1, size_s);
    curr_i++;
    if(report) {
        reports_s[counter] = report;
        counter++;
    }
    pthread_mutex_unlock(&mutex);
}

void* thread_func(void *argv)
{
    char *url = (char *) argv;
    Report *report = scan_server(url);
    finish_scanning(report);
    return NULL;
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
            result = -1;
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

    reports_s = malloc(op.size * sizeof(Report *));
    pthread_t* threads = malloc(op.size * sizeof (pthread_t));
    pthread_mutex_init(&mutex, NULL);
    puts("Start scanning...");
    size_s = op.size;
    for (int i = 0; i < op.size; i++ )
    {
        pthread_create(&threads[i], NULL, thread_func, *(op.servers + i));
    }

    for (int i = 0; i < op.size; i++ )
    {
        pthread_join(threads[i], NULL);
    }

    if(op.file_o)
    {
        fp = freopen(op.file_o, "wb", stdout);
        if(fp == NULL) {
            printf("Error in open file %s", op.file_o);
            result = -1;
            goto free_;
        }
        printReports(reports_s, size_s);
        fclose(fp);
    }else
    {
        printReports(reports_s, size_s);
    }

    free_:
    if(op.file_f)
    {
        for(int i = 0; i < op.size; i++)
            free(op.servers[i]);
        free(op.servers);
    }
    pthread_mutex_destroy(&mutex);
    free(threads);
    if(reports_s) {
        for (int i = 0; i < size; i++)
                free(reports_s[i]);
        free(reports_s);
    }
    return result;
}
