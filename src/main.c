#include "scheduler.h"
#include "time.h"
#include "input.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

ISR (TIMER1_OVF_vect)
{
    timer_manager_timer1_ovf_handler();
}

ISR(PCINT2_vect)
{
    input_manager_pin_change_handler();
}

static void task1_main(void)
{
    DDRC |= 1;
    PORTC &= ~1;

    while (1)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            PORTC ^= 1;
            await_sleep(75);
        }
        await_sleep(1000);
    }
}

static void task2_main(void)
{
    DDRC |= 2;
    while (1)
    {
        Gesture press = await_input(Button0);
        uint16_t time = (press == ShortPress) ? 200 : 1000;
        PORTC |= 2;
        await_sleep(time);
        PORTC &= ~2;
    }
}

int main(void)
{
    input_manager_init_hardware();
    timer_manager_init_hardware();

    InputManager input_manager;
    input_manager_init(&input_manager);
    input_manager_set_global_inst(&input_manager);

    TimerManager timer_manager;
    timer_manager_init(&timer_manager);
    timer_manager_set_global_inst(&timer_manager);

    Scheduler scheduler;
    scheduler_init(&scheduler);

    Thread thread_t1, thread_t2;
    thread_init(&thread_t1, task1_main);
    thread_init(&thread_t2, task2_main);

    scheduler_register_thread(&scheduler, &thread_t1);
    scheduler_register_thread(&scheduler, &thread_t2);

    sei();

    scheduler_run(&scheduler);
}