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

void* output(void *arg)
{
    OutThreadPoolArg *out_data = (OutThreadPoolArg *) arg;
    printf("%d of %d\n", out_data->out_data->count + 1, out_data->out_data->size);
    out_data->out_data->count++;
    if(out_data->report)
    {
        *out_data->out_data->reports = out_data->report;
        out_data->out_data->reports++;
    }
    else
    {
        fprintf(stderr, "Error in target %s : %s\n", out_data->url, out_data->error);
    }
    if(out_data->out_data->count >= out_data->out_data->size )
        stop(out_data->ctx);
    free(out_data);
    return NULL;
}

void* func_thread(void* data, IO_ctx *ctx)
{
    InThreadPoolArg *in_data = (InThreadPoolArg*) data;
    char *err;
    Report *report = scan_server_with_error(in_data->url, &err);
    OutThreadPoolArg *out_data = malloc(sizeof(OutThreadPoolArg));
    out_data->out_data = in_data->out_data;
    out_data->report = report;
    out_data->url = in_data->url;
    out_data->ctx = in_data->ctx;
    out_data->error = err;
    push(ctx, out_data);
    free(in_data);
    return NULL;
}

int threading_visit_with_thread_pool(int size, char **urls, Report **out_reports)
{
    ThreadConst threadConst = {.reports = out_reports, .count = 0, .size = size};
    InThreadPoolArg *in_arg = NULL;
    Thread_ctx *ctx = init(COUNT_THREAD, func_thread);
    set_output(ctx, output);
    puts("Start scanning...");

    for(int i = 0; i < size; ++i)
    {
        in_arg = malloc(sizeof(InThreadPoolArg));
        in_arg->url = urls[i];
        in_arg->ctx = ctx;
        in_arg->out_data = &threadConst;
        push_data(ctx, in_arg);
        in_arg = NULL;
    }
    run(ctx);
    while (is_alive(ctx))
    {
    }
//    while (threadConst.count <= threadConst.size - 1) //tut minus kotic
//    {
//    }
//    stop(ctx);
    destroy(ctx);
    return (int)(threadConst.reports - out_reports);
}
