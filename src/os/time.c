#include "time.h"

#include "atomic.h"
#include "scheduler.h"

#include <avr/io.h>
#include <assert.h>

static TimerManager *timer_manager_inst = 0;

void timer_manager_init_hardware(void)
{
    // Use Timer0 overflow interrupt for system ticks
    TCCR0B |= (1 << CS01);  // Prescale of 8
    TIMSK0 |= (1 << TOIE0);  // Interrupt on overflow (8bit)
}

void timer_manager_init(TimerManager *inst)
{
    inst->system_time_ticks = 0;
    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
    {
        inst->pipes[i] = 0;
        inst->countdown[i] = 0;
    }
}

void timer_manager_set_global_inst(TimerManager *inst)
{
    timer_manager_inst = inst;
}

static void timer_request(Pipe *pipe, uint32_t ticks)
{
    uint8_t i;
    for (i = 0; i < MAX_TIMERS; i++)
    {
        if (timer_manager_inst->pipes[i] == 0)
        {
            ATOMIC({
                timer_manager_inst->countdown[i] = ticks;
                timer_manager_inst->pipes[i] = pipe;
            });
            break;
        }
    }
    assert(i < MAX_TIMERS);
}

void await_sleep(uint32_t ticks)
{
    Pipe pipe;
    pipe_init(&pipe);
    timer_request(&pipe, ticks);
    scheduler_await(&pipe);
}

void timer_manager_timer0_ovf_handler(void)
{
    if (!timer_manager_inst)
    {
        return;
    }

    ++timer_manager_inst->system_time_ticks;
    for (uint8_t i = 0; i < MAX_TIMERS; ++i)
    {
        if (timer_manager_inst->pipes[i] != 0)
        {
            uint16_t count = --timer_manager_inst->countdown[i];
            if (count == 0)
            {
                timer_manager_inst->pipes[i]->ready_bool = 1;
                timer_manager_inst->pipes[i] = 0;
            }
        }
    }
}

uint32_t system_time_ticks(void)
{
    if (timer_manager_inst)
    {
        return timer_manager_inst->system_time_ticks;
    }
    return 0;
}
