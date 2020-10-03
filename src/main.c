#include "os/input.h"
#include "os/ir.h"
#include "os/led.h"
#include "os/light_sensor.h"
#include "os/scheduler.h"
#include "os/time.h"

#include "app/led_anim.h"

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

static void task1_main(void *param)
{
    const LedNumber my_led = Led0;

    LedOpCode blink1_seq[LED_ANIM_MAX_PROGRAM_LEN], blinkn_seq[LED_ANIM_MAX_PROGRAM_LEN];
    led_anim_sequence_pulse_then_off(blink1_seq, 1, MS_TO_ANIM_TICKS(100));
    led_anim_sequence_pulse_then_off(blinkn_seq, 3, MS_TO_ANIM_TICKS(100));

    while (1)
    {
        Gesture press = await_input(Button0);
        if (press == ShortPress)
        {
            ir_play(IRPort0, ir_buffer_contents(), ir_buffer_length());
            ir_play(IRPort1, ir_buffer_contents(), ir_buffer_length());
            led_anim_play(my_led, blink1_seq, LED_ANIM_MAX_PROGRAM_LEN);
        }
        else
        {
            ir_buffer_clear();
            led_anim_play(my_led, blinkn_seq, LED_ANIM_MAX_PROGRAM_LEN);
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

static void task2_main(void *param)
{
    const LedNumber my_led = Led1;

    BinaryAverage avg = {0, 0, 0};
    while(1)
    {
        char cur_sensor = read_max_light_sensor() == Sensor1 ? 0 : 1;
        char avg_sensor = compute_rolling_avg(cur_sensor, &avg);
        led_set_state(my_led, avg_sensor);

        await_sleep(MS_TO_TICKS(100 / 16));
    }
}

int main(void)
{
    input_manager_init_hardware();
    timer_manager_init_hardware();
    ir_init_hardware();
    led_init_hardware();

    LedAnimManager led_anim;
    led_anim_init(&led_anim);
    led_anim_set_global_inst(&led_anim);

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

    Thread thread_led_anim;
    thread_init(&thread_led_anim, led_anim_thread, (void*)&led_anim);
    scheduler_register_thread(&scheduler, &thread_led_anim);

    Thread t1, t2;
    thread_init(&t1, task1_main, 0);
    thread_init(&t2, task2_main, 0);
    scheduler_register_thread(&scheduler, &t1);
    scheduler_register_thread(&scheduler, &t2);

    // testing - fancy counter blink
    LedOpCode blink_seq[LED_ANIM_MAX_PROGRAM_LEN];
    uint8_t blink_seq_len = led_anim_sequence_infinite_pulse(blink_seq, 3, MS_TO_ANIM_TICKS(100), MS_TO_ANIM_TICKS(1000));
    led_anim_play(Led2, blink_seq, blink_seq_len);

    sei();

    scheduler_run(&scheduler);
}