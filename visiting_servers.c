//
// Created by tntrol on 17.08.2021.
//

#include "visiting_servers.h"
#include "scanner.h"
//#define DEBUG


void finish_scanning(ThreadConst *threadConst, Report *report, char *err, char *url)
{
    pthread_mutex_lock(&threadConst->mutex);
    int size = threadConst->size;
    printf("%d of %d\n", threadConst->count + 1, size);
    threadConst->count++;
    if(report)
    {
        *(threadConst->reports) = report;
        threadConst->reports++;
    }else if(err)
    {
        //fputs( err, stderr);
        fprintf(stderr, "Error in target %s : %s\n", url, err);
    }
    pthread_mutex_unlock(&threadConst->mutex);
}

void* thread_func(void *argv)
{
    ThreadArg *threadArg= (ThreadArg *) argv;
    char *err;
#ifdef DEBUG1
    printf("URL_THREAD = %s\n", threadArg->url);
#endif
    Report *report = scan_server_with_error(threadArg->url, &err);
    finish_scanning(threadArg->thread_data, report, err, threadArg->url);
    free(threadArg);
    return NULL;
}

int threading_visit(int size, char **urls, Report **out_reports)
{
    pthread_t* threads = malloc(size * sizeof (pthread_t));
    ThreadConst threadConst = {.reports = out_reports, .count = 0, .size = size};
    ThreadArg *threadArg = NULL;

    pthread_mutex_init(&threadConst.mutex, NULL);
    puts("Start scanning...");
    for (int i = 0; i < size; i++ )
    {
        threadArg = malloc(sizeof(ThreadArg));
        threadArg->url = *(urls + i);
        threadArg->thread_data = &threadConst;
        pthread_create(&threads[i], NULL, thread_func, threadArg);
        threadArg = NULL;
    }
    for (int i = 0; i < size; i++ )
    {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&threadConst.mutex);
    free(threads);
#ifdef DEBUG
    printf("SIZE = %d\n", threadConst.reports - out_reports);
#endif
    return (int)(threadConst.reports - out_reports);
}

int serial_visit(int size, char **urls, Report **out_reports)
{
    int read = 0;
    puts("Start scanning...");
    for (int i = 0; i < size; i++ )
    {
        Report *report = scan_server(*(urls + i));
        if(report)
        {
            out_reports[read] = report;
            read++;
        }
        printf("%d of %d\n", i + 1, size);
    }
    return read;
}
