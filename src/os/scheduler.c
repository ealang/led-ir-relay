#include "scheduler.h"

#include <assert.h>
#include <string.h>

#define FRAME_BACKUP_SIZE 19  // Bytes needed to preserve current stack frame
#define THREAD_ID_SCHEDULER 0
#define THREAD_ID_USER_START 1

#define HIGH(x) ((uint16_t)(x) >> 8)
#define LOW(x) ((uint16_t)(x) & 0xFF)

extern void _thread_startup_wrapper(void *param);
extern void _scheduler_resume_thread(Thread *to);
extern void _scheduler_switch_threads(Thread *from, Thread *to);

static Scheduler *scheduler_inst = 0;
static uint8_t current_thread = 0;

// Switch to the given thread and suspect current thread.
static void scheduler_switch_threads(uint8_t thread_id)
{
    Thread *starting_thread = scheduler_inst->threads[current_thread];
    Thread *ending_thread = scheduler_inst->threads[thread_id];
    current_thread = thread_id;
    _scheduler_switch_threads(starting_thread, ending_thread);
}

// Thread main that will schedule other threads.
static void scheduler_main(void *param)
{
    // Start all the threads
    for (uint8_t i = THREAD_ID_USER_START; i < MAX_THREADS; i++)
    {
        if (scheduler_inst->threads[i] != 0)
        {
            scheduler_switch_threads(i);
        }
    }

    while (1)
    {
        for (uint8_t i = THREAD_ID_USER_START; i < MAX_THREADS; i++)
        {
            Pipe *pipe = scheduler_inst->pipes[i];
            if (pipe != 0 && pipe->ready_bool)
            {
                scheduler_inst->pipes[i] = 0;
                scheduler_switch_threads(i);
            }
        }
    }
}

void pipe_init(Pipe *pipe)
{
    pipe->ready_bool = 0;
}

void thread_init(Thread *thread, void (*start)(void*), void *param)
{
    const uint8_t startup_param_size = 6;  // storage for extra data pushed below

    // Init thread stack
    memset((void *)thread->stack, 0, THREAD_STACK_SIZE);
    thread->sp = thread->stack + THREAD_STACK_SIZE - FRAME_BACKUP_SIZE - 1 - startup_param_size;

    // Thread wrapper addr
    thread->stack[THREAD_STACK_SIZE - 6] = HIGH(_thread_startup_wrapper);
    thread->stack[THREAD_STACK_SIZE - 5] = LOW(_thread_startup_wrapper);

    // User thread param
    thread->stack[THREAD_STACK_SIZE - 4] = HIGH(param);
    thread->stack[THREAD_STACK_SIZE - 3] = LOW(param);

    // User func
    thread->stack[THREAD_STACK_SIZE - 2] = HIGH(start);
    thread->stack[THREAD_STACK_SIZE - 1] = LOW(start);
}

void scheduler_init(Scheduler *scheduler)
{
    for (uint8_t i = 0; i < MAX_THREADS; ++i)
    {
        scheduler->threads[i] = 0;
        scheduler->pipes[i] = 0;
    }
    thread_init(&scheduler->scheduler_thread, scheduler_main, 0);
    scheduler_register_thread(scheduler, &scheduler->scheduler_thread);
}

uint8_t scheduler_register_thread(Scheduler *scheduler, Thread *thread)
{
    // Register with scheduler
    uint8_t i;
    for (i = 0; i < MAX_THREADS; ++i)
    {
        if (scheduler->threads[i] == 0)
        {
            scheduler->threads[i] = thread;
            break;
        }
    }
    assert (i < MAX_THREADS);

    return i;
}

void scheduler_run(Scheduler *scheduler)
{
    current_thread = THREAD_ID_SCHEDULER;
    scheduler_inst = scheduler;

    Thread *thread = scheduler_inst->threads[THREAD_ID_SCHEDULER];
    _scheduler_resume_thread(thread);
}

void scheduler_await(Pipe *pipe)
{
    assert(scheduler_inst->pipes[current_thread] == 0);
    scheduler_inst->pipes[current_thread] = pipe;
    scheduler_switch_threads(THREAD_ID_SCHEDULER);
}
