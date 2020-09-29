#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define MAX_THREADS 5
#define THREAD_STACK_SIZE 0x100

typedef union
{
    uint8_t byte;
    uint16_t word;
} PipeData;

// Synchronization struct
typedef struct Pipe
{
    char ready_bool;  // set to 1 when data ready to be consumed
    PipeData value;
} Pipe;

typedef struct
{
    uint8_t *sp;
    uint8_t stack[THREAD_STACK_SIZE];
} Thread;

typedef struct
{
    Thread *threads[MAX_THREADS];
    Pipe *pipes[MAX_THREADS];
    Thread scheduler_thread;
} Scheduler;

void pipe_init(Pipe *pipe);
void thread_init(Thread *thread, void (*start)(void));
void scheduler_init(Scheduler *scheduler);

// Register the given thread with the scheduler.
// @return Thread id
uint8_t scheduler_register_thread(Scheduler *scheduler, Thread *thread);

// Turn control over to the scheduler.
void scheduler_run(Scheduler *scheduler);

// Suspend current thread until the given pipe has a value. 
void scheduler_await(Pipe *pipe);

#endif