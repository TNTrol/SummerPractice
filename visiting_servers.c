//
// Created by tntrol on 17.08.2021.
//

#include "visiting_servers.h"
#include "scanner.h"
#include "thread_pool.h"

#define COUNT_THREAD 3
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
    }
    else if(err)
    {
        fprintf(stderr, "Error in target %s : %s\n", url, err);
    }
    pthread_mutex_unlock(&threadConst->mutex);
}

void* thread_func(void *argv)
{
    ThreadArg *threadArg= (ThreadArg *) argv;
    char *err;
#ifdef DEBUG
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
    for(int i = 0; i < size; ++i)
    {
        threadArg = malloc(sizeof(ThreadArg));
        threadArg->url = *(urls + i);
        threadArg->thread_data = &threadConst;
        pthread_create(&threads[i], NULL, thread_func, threadArg);
        threadArg = NULL;
    }
    for(int i = 0; i < size; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&threadConst.mutex);
    free(threads);
    return (int)(threadConst.reports - out_reports);
}

int serial_visit(int size, char **urls, Report **out_reports)
{
    int read = 0;
    puts("Start scanning...");
    for(int i = 0; i < size; ++i)
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

void* func_thread1(Thread_ctx *ctx, void* arg)
{
    ThreadArg *threadArg = (ThreadArg*) arg;
    char *err = NULL;
    Report *report = scan_server_with_error(threadArg->url, &err);
    pthread_mutex_lock(&threadArg->thread_data->mutex);
    printf("%d of %d\n", threadArg->thread_data->count + 1, threadArg->thread_data->size);
    threadArg->thread_data->count++;
    if(report)
    {
        *threadArg->thread_data->reports = report;
        threadArg->thread_data->reports++;
    }
    else
    {
        fprintf(stderr, "Error in target %s : %s\n", threadArg->url, err);
    }
    if(threadArg->thread_data->count >= threadArg->thread_data->size)
    {
        stop(ctx);
    }
    pthread_mutex_unlock(&threadArg->thread_data->mutex);
    free(arg);
    return NULL;
}

void* loop_func(Thread_ctx *ctx, void *arg)
{
    ThreadAllData *data = (ThreadAllData*) arg;
    ThreadArg *arg_t = NULL;
    int size;
    if (data->url)
    {
        size = data->ctx->size;
        while (size)
        {
            arg_t = malloc(sizeof(ThreadArg));
            arg_t->url = *(data->url++);
            arg_t->thread_data = data->ctx;
            push_in_pool(ctx, func_thread1, arg_t);

            arg_t = NULL;
            size--;
        }
        data->url = NULL;
    }
    return NULL;
}

int threading_visit_with_thread_pool(int size, char **urls, Report **out_reports)
{
    ThreadConst threadConst = {.reports = out_reports, .count = 0, .size = size};
    ThreadAllData threadAllData = {.url = urls, .ctx = &threadConst};
    Thread_ctx *ctx = init(COUNT_THREAD, loop_func, &threadAllData);
    puts("Start scanning...");
    run(ctx);
    while (is_alive(ctx))
    {
    }
    destroy(ctx);
    return (int)(threadConst.reports - out_reports);
}
