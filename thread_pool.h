//
// Created by tntrol on 23.08.2021.
//

#ifndef SUMMERPRACTICE_THREAD_POOL_H
#define SUMMERPRACTICE_THREAD_POOL_H

struct thread_ctx;
struct io_ctx;

struct message{
    void *data;
    struct message *next;
};

enum Status{
    CRITICAL_STOP = -1,
    STOP = 0,
    RUN = 1
};

typedef struct io_ctx IO_ctx;
typedef struct message Message;
typedef struct thread_ctx Thread_ctx;

// T_T
void push(IO_ctx *ctx, void *);
void push_data(Thread_ctx *ctx, void* data);
Thread_ctx * init( int count_thread, void* (*thread_function) (void*, IO_ctx *));
void run(Thread_ctx *ctx);
void stop(Thread_ctx *ctx);
void destroy(Thread_ctx *ctx);
int set_input(Thread_ctx *ctx, void* (*i_function) (IO_ctx *));
int set_output(Thread_ctx *ctx, void* (*o_function) (void*));
int is_alive(Thread_ctx *ctx);

#endif //SUMMERPRACTICE_THREAD_POOL_H
