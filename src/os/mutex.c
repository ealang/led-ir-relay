#include "mutex.h"
#include "scheduler.h"

#include <assert.h>
#include <string.h>

void mutex_init(Mutex *mutex)
{
    memset(mutex, 0, sizeof(Mutex));
}

void await_mutex_acquire(Mutex *mutex)
{
    if (!mutex->taken)
    {
        mutex->taken = 1;
        return;
    }

    uint8_t i = (mutex->next_awaiter + mutex->num_awaiters++) % MUTEX_MAX_CLIENTS;
    assert(mutex->num_awaiters <= MUTEX_MAX_CLIENTS);

    Pipe pipe;
    pipe_init(&pipe);
    mutex->awaiters[i] = &pipe;

    scheduler_await(&pipe);
}

void mutex_release(Mutex *mutex)
{
    assert (mutex->taken);
    if (mutex->num_awaiters == 0)
    {
        mutex->taken = 0;
        return;
    }

    --mutex->num_awaiters;

    uint8_t i = mutex->next_awaiter;
    mutex->next_awaiter = (i + 1) % MUTEX_MAX_CLIENTS;

    Pipe *next_awaiter = mutex->awaiters[i];
    mutex->awaiters[i] = 0;
    assert (next_awaiter);

    next_awaiter->ready_bool = 1;
}
