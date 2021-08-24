//
// Created by tntrol on 23.08.2021.
//

#include "thread_pool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct io_ctx{
    Message *first_message;
    Message *last_message;
    pthread_mutex_t mutex;
    void* (*io_function) (void*);
    pthread_t thread;
    int size;
};

struct thread_ctx{
    IO_ctx *input_ctx;
    IO_ctx *output_ctx;
    int count_thread;
    pthread_t *threads;
    void* (*thread_function) (void*, IO_ctx*);
    enum Status status;
};

void push(IO_ctx *ctx, void *data)
{
    pthread_mutex_lock(&ctx->mutex);
    Message *message = malloc(sizeof(Message));
    message->data = data;
    message->next = NULL;
    ctx->size++;
    if(ctx->first_message == NULL)
    {
        ctx->first_message = message;
        ctx->last_message = message;
    }
    else
    {
        ctx->last_message->next = message;
        ctx->last_message = message;
    }
    pthread_mutex_unlock(&ctx->mutex);
}

int pop(IO_ctx *out, Message *out_message)
{
    int res = 0;
    pthread_mutex_lock(&out->mutex);
    Message *m = out->first_message;
    if(m)
    {
        out_message->data = m->data;
        out->first_message = out->first_message->next;
        res = 1;
        out->size--;
        if(out->first_message == NULL)
        {
            out->last_message = NULL;
            out->size = 0;
        }
    }
    free(m);
    pthread_mutex_unlock(&out->mutex);
    return res;
}

void* o_thread(void *arg)
{
    Thread_ctx *ctx = (Thread_ctx*) arg;
    Message message = {.data = NULL, .next = NULL};
    int res = 0;
    while(ctx->status == RUN || ctx->input_ctx->size > 0)
    {
        res = pop(ctx->output_ctx, &message);
        if(res)
        {
            ctx->output_ctx->io_function(message.data);
        }
    }
    return NULL;
}

void* i_thread(void *arg)
{
    Thread_ctx *ctx = (Thread_ctx*) arg;
    while (ctx->status == RUN)
    {
        ctx->input_ctx->io_function(ctx->input_ctx);
    }
    return NULL;
}

int size_stack(IO_ctx *ctx)
{
    int r = 0;
    pthread_mutex_lock(&ctx->mutex);
    r = ctx->size;
    //printf("SIZE = %d\n", r);
    pthread_mutex_unlock(&ctx->mutex);
    return r;
}

void* thread_functions(void* arg)
{
    Thread_ctx *ctx = (Thread_ctx*) arg;
    Message message = {.data = NULL, .next = NULL};
    int res = 0;
    while ( ctx->status == RUN || res > 0 )
    {
        res = pop(ctx->input_ctx, &message);
        if(res)
        {
            ctx->thread_function(message.data, ctx->output_ctx);
        }
    }
    return NULL;
}

IO_ctx * init_io_ctx()
{
    IO_ctx *ctx = malloc(sizeof(IO_ctx));
    ctx->first_message = NULL;
    ctx->last_message = NULL;
    ctx->size = 0;
    ctx->io_function = NULL;
    pthread_mutex_init(&ctx->mutex, NULL);
    return ctx;
}

Thread_ctx *init(int count_thread, void* (*thread_function) (void*, IO_ctx*))
{
    Thread_ctx *ctx = malloc(sizeof(Thread_ctx));
    ctx->count_thread = count_thread;
    ctx->status = STOP;
    ctx->input_ctx = init_io_ctx();
    ctx->output_ctx = init_io_ctx();
    ctx->thread_function = thread_function;
    ctx->threads = malloc(count_thread * sizeof (pthread_t));
    return ctx;
}

void run(Thread_ctx *ctx)
{
    ctx->status = RUN;
    if(ctx->input_ctx->io_function)
    {
        pthread_create(&ctx->input_ctx->thread, NULL, i_thread, ctx);
    }
    if(ctx->output_ctx->io_function)
    {
        pthread_create(&ctx->output_ctx->thread, NULL, o_thread, ctx);
    }
    for(int i = 0; i < ctx->count_thread; ++i)
    {
        pthread_create(&ctx->threads[i], NULL, thread_functions, ctx);
    }
}

void stop(Thread_ctx *ctx)
{
    while (size_stack(ctx->input_ctx) > 0)
    {
    }
    ctx->status = STOP;
    if(ctx->input_ctx->io_function)
    {
        pthread_join(ctx->input_ctx->thread, NULL);
    }
    for(int i = 0; i < ctx->count_thread; ++i)
    {
        pthread_join(ctx->threads[i], NULL);
    }
    if(ctx->output_ctx->io_function)
    {
        pthread_join(ctx->output_ctx->thread, NULL);
    }
}

void free_io_ctx(IO_ctx *ctx)
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
    free_io_ctx(ctx->output_ctx);
    free_io_ctx(ctx->input_ctx);
    free(ctx->threads);
}

void push_data(Thread_ctx *ctx, void *data)
{
    push(ctx->input_ctx, data);
}

int set_output(Thread_ctx *ctx, void* (*o_function) (void*))
{
    if(ctx->status == RUN)
    {
        return -1;
    }
    ctx->output_ctx->io_function = o_function;
    return 1;
}

int set_input(Thread_ctx *ctx, void* (*i_function) (IO_ctx *))
{
    if(ctx->status == RUN)
    {
        return -1;
    }
    ctx->input_ctx->io_function = (void *(*)(void *)) i_function;
    return 1;
}

int is_alive(Thread_ctx *ctx)
{
    return ctx->status == RUN;
}
