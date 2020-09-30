#ifndef TIME_H_
#define TIME_H_

#include <stdint.h>

typedef struct Pipe Pipe;

#define MAX_TIMERS 5

typedef struct {
    uint16_t system_time_ticks;
    Pipe *pipes[MAX_TIMERS];
    uint16_t countdown[MAX_TIMERS];
} TimerManager;

void timer_manager_init_hardware(void);

// Handler for TIMER1_OVF interrupt
void timer_manager_timer1_ovf_handler(void);

void timer_manager_init(TimerManager *inst);
void timer_manager_set_global_inst(TimerManager *inst);

// Thread aware sleep
void await_sleep(uint16_t ms);

// Time since startup. Wraps on overflow.
uint16_t system_time_ticks(void);

#define SEC_PER_CLOCK_TICK 0.008192

#endif