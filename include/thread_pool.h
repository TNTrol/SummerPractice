//
// Created by tntrol on 23.08.2021.
//

#ifndef SUMMERPRACTICE_THREAD_POOL_H
#define SUMMERPRACTICE_THREAD_POOL_H

struct thread_ctx;
struct queue;
struct message;

enum Status{
    CRITICAL_STOP = -1,
    STOP = 0,
    RUN = 1
};

typedef struct queue Queue;
typedef struct message Message;
typedef struct thread_ctx Thread_ctx;

// T_T
void push_in_pool(Thread_ctx *ctx, void* (*thread_function) (Thread_ctx *, void*), void *arg);
Thread_ctx * init( int count_thread, void* (*thread_function) (Thread_ctx *, void*), void *arg);
void run(Thread_ctx *ctx);
void stop(Thread_ctx *ctx);
void destroy(Thread_ctx *ctx);
int is_alive(Thread_ctx *ctx);

#endif //SUMMERPRACTICE_THREAD_POOL_H
