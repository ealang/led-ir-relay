#ifndef TIME_H_
#define TIME_H_

#include <stdint.h>

typedef struct Pipe Pipe;

#define MAX_TIMERS 5

typedef struct {
    uint32_t system_time_ticks;
    Pipe *pipes[MAX_TIMERS];
    uint32_t countdown[MAX_TIMERS];
} TimerManager;

void timer_manager_init_hardware(void);

// Handler for TIMER0_OVF interrupt
void timer_manager_timer0_ovf_handler(void);

void timer_manager_init(TimerManager *inst);
void timer_manager_set_global_inst(TimerManager *inst);

#define MS_TO_TICKS(ms) ((uint32_t)(3.90625 * ms)) // 8000000 / (8 * 256 * 1000) * ms

// Thread aware sleep
void await_sleep(uint32_t ticks);

// Time since boot in ticks.
uint32_t system_time_ticks(void);

#endif