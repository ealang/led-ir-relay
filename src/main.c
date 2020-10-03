#include "input.h"
#include "ir.h"
#include "light_sensor.h"
#include "scheduler.h"
#include "time.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

ISR (TIMER0_OVF_vect)
{
    timer_manager_timer0_ovf_handler();
}

ISR(PCINT0_vect)
{
    ir_receive_handler();
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
        uint8_t len = ir_buffer_length();

        PORTC |= 1;
        if (len < IR_BUFFER_SIZE)
        {
            for (uint8_t i = 0; i < 3; i++)
            {
                await_sleep(MS_TO_TICKS(70));
                PORTC ^= 1;
            }
        }

        await_sleep(MS_TO_TICKS(1000));
    }
}

static void task2_main(void)
{
    while (1)
    {
        Gesture press = await_input(Button0);
        if (press == ShortPress)
        {
            ir_play(IRPort0, ir_buffer_contents(), ir_buffer_length());
            ir_play(IRPort1, ir_buffer_contents(), ir_buffer_length());
        }
        else
        {
            ir_buffer_clear();
        }
    }
}

typedef struct {
    uint8_t count;
    uint8_t sum;
    uint16_t history;
} BinaryAverage;

char compute_rolling_avg(char bit, BinaryAverage *inst);
char compute_rolling_avg(char bit, BinaryAverage *inst)
{
    char oldest_bit = inst->history >> 15;
    inst->sum += bit - oldest_bit;

    inst->history = (inst->history << 1) | bit;
    if (inst->count < 16)
    {
        ++inst->count;
        // not enough history yet, return current
        return bit;
    }
    return (inst->sum >= 8) ? 1 : 0;
}

static void task3_main(void)
{
    DDRC |= 2;

    BinaryAverage avg = {0, 0, 0};
    while(1)
    {
        char cur_sensor = read_max_light_sensor() == Sensor1 ? 0 : 1;
        char avg_sensor = compute_rolling_avg(cur_sensor, &avg);

        PORTC = (PORTC & ~2) | (avg_sensor << 1);

        await_sleep(MS_TO_TICKS(100 / 16));
    }
}

#include <stdio.h>

int main(void)
{
    input_manager_init_hardware();
    timer_manager_init_hardware();
    ir_init_hardware();

    InputManager input_manager;
    input_manager_init(&input_manager);
    input_manager_set_global_inst(&input_manager);

    TimerManager timer_manager;
    timer_manager_init(&timer_manager);
    timer_manager_set_global_inst(&timer_manager);

    IRBuffer ir_buffer;
    ir_buffer_init(&ir_buffer);
    ir_buffer_set_global_inst(&ir_buffer);

    Scheduler scheduler;
    scheduler_init(&scheduler);

    Thread thread_t1, thread_t2, thread_t3;
    thread_init(&thread_t1, task1_main);
    thread_init(&thread_t2, task2_main);
    thread_init(&thread_t3, task3_main);

    scheduler_register_thread(&scheduler, &thread_t1);
    scheduler_register_thread(&scheduler, &thread_t2);
    scheduler_register_thread(&scheduler, &thread_t3);

    sei();

    scheduler_run(&scheduler);
}