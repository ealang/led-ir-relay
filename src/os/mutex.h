#ifndef MUTEX_H_
#define MUTEX_H_

#include <stdint.h>

typedef struct Pipe Pipe;

#define MUTEX_MAX_CLIENTS 5
typedef struct Mutex
{
    char taken;
    Pipe *awaiters[MUTEX_MAX_CLIENTS];
    uint8_t next_awaiter;
    uint8_t num_awaiters;
} Mutex;

void mutex_init(Mutex *mutex);

// Block until mutex is available. Thread-aware.
void await_mutex_acquire(Mutex *mutex);

void mutex_release(Mutex *mutex);

#endif