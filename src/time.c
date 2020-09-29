#include "time.h"
#include "scheduler.h"

#include <avr/io.h>
#include <assert.h>

#define CLOCK_TICKS_PER_SEC 122

static TimerManager *timer_manager_inst = 0;

static uint8_t max(uint8_t a, uint8_t b)
{
    if (a > b)
    {
        return a;
    }
    return b;
}

void timer_manager_init_hardware(void)
{
    // Use Timer1 overflow interrupt for sleeps
    TCCR1B |= (1 << CS10);  // Prescale of 1
    TIMSK1 |= (1 << TOIE1);  // Interrupt on overflow (16bit)
}

void timer_manager_init(TimerManager *inst)
{
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

static void timer_request(Pipe *pipe, uint16_t ms)
{
    uint8_t i;
    for (i = 0; i < MAX_TIMERS; i++)
    {
        if (timer_manager_inst->pipes[i] == 0)
        {
            timer_manager_inst->countdown[i] = max(ms >> 3, 1);  // approx conversion to clock ticks
            timer_manager_inst->pipes[i] = pipe;  // set this last in case ISR runs
            break;
        }
    }
    assert(i < MAX_TIMERS);
}

void await_sleep(uint16_t ms)
{
    Pipe pipe;
    pipe_init(&pipe);
    timer_request(&pipe, ms);
    scheduler_await(&pipe);
}

void timer_manager_timer1_ovf_handler(void)
{
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
