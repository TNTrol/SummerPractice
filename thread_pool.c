//
// Created by tntrol on 23.08.2021.
//

#include "thread_pool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct message{
    void* (*thread_function) (Thread_ctx *, void*);
    void *arg;
    Message *next;
};

struct queue{
    Message *first_message;
    Message *last_message;
    pthread_mutex_t mutex;
};

struct thread_ctx{
    Queue *queue;
    pthread_t *threads;
    void *arg;
    void* (*thread_function) (Thread_ctx *, void*);
    int count_thread;
    enum Status status;
};

void *loop(void *arg)
{
    Thread_ctx *ctx = (Thread_ctx*) arg;
    while (ctx->status == RUN)
    {
        ctx->thread_function(ctx, ctx->arg);
    }
    return NULL;
}

Message* pop(Queue *queue)
{
    Message *message = NULL;
    pthread_mutex_lock(&queue->mutex);
    if(queue->first_message)
    {
        message = queue->first_message;
        queue->first_message = message->next;
        if(message->next == NULL)
        {
            queue->last_message = NULL;
        }
        message->next = NULL;
    }
    pthread_mutex_unlock(&queue->mutex);
    return message;
}

void *thread_function(void* arg)
{
    Thread_ctx *ctx = (Thread_ctx*) arg;
    Message *message;
    while (ctx->status == RUN)
    {
        message = pop(ctx->queue);
        if(message)
        {
            message->thread_function(ctx, message->arg);
        }
        free(message);
        message = NULL;
    }
    return NULL;
}

Thread_ctx *init(int count_thread, void* (*thread_function) (Thread_ctx *, void*), void *arg)
{
    Thread_ctx *ctx = malloc(sizeof(Thread_ctx));
    ctx->status = STOP;
    ctx->thread_function = thread_function;
    ctx->count_thread = count_thread;
    ctx->arg = arg;
    ctx->queue = malloc(sizeof(Queue));
    ctx->queue->first_message = NULL;
    ctx->queue->last_message = NULL;
    pthread_mutex_init(&ctx->queue->mutex, NULL);
    ctx->threads = malloc(sizeof(pthread_t) * count_thread + 1);
    return ctx;
}

void run(Thread_ctx *ctx)
{
    ctx->status = RUN;
    pthread_create(&ctx->threads[0], NULL, loop, ctx);

    for(int i = 1; i <= ctx->count_thread; ++i)
    {
        pthread_create(&ctx->threads[i], NULL, thread_function, ctx);
    }

}

void stop_thread(Thread_ctx *ctx)
{
    ctx->status = STOP;
    for(int i = 0; i <= ctx->count_thread; ++i)
    {
        pthread_join(ctx->threads[i], NULL);
    }
}

void free_io_ctx(Queue *ctx)
{
    Message *msg = NULL;
    while(ctx->first_message)
    {
        msg = ctx->first_message;
        ctx->first_message = ctx->first_message->next;
        free(msg);
    }
    pthread_mutex_destroy(&ctx->mutex);
}

void destroy(Thread_ctx *ctx)
{
    if(ctx->status == RUN)
    {
        return;
    }
    stop_thread(ctx);
    free_io_ctx(ctx->queue);
    free(ctx->threads);
}

int is_alive(Thread_ctx *ctx)
{
    return ctx->status == RUN;
}

void stop(Thread_ctx *ctx)
{
    ctx->status = STOP;
}

void push_in_pool(Thread_ctx *ctx, void *(*thread_function)(Thread_ctx *, void *), void *arg)
{
    Message *message = malloc(sizeof(Message));
    message->arg = arg;
    message->thread_function = thread_function;
    pthread_mutex_lock(&ctx->queue->mutex);
    if(!ctx->queue->first_message)
    {
        ctx->queue->first_message = message;
    }
    else
    {
        ctx->queue->last_message->next = message;
    }
    ctx->queue->last_message = message;
    pthread_mutex_unlock(&ctx->queue->mutex);
}