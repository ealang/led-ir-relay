#include "os/input.h"
#include "os/ir.h"
#include "os/led.h"
#include "os/scheduler.h"
#include "os/time.h"
#include "led_anim/led_anim.h"
#include "app/kvm_switch_task.h"

#include <avr/io.h>
#include <avr/interrupt.h>

extern void ui_task(void *param);

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

    // Switch params
    KvmSwitchTaskCtrl kvm_switch_task_ctrl = {
        // default to auto-select port
        .port_selection = KVM_SWITCH_PORT_AUTO
    };

    Thread t1, t2;
    thread_init(&t1, ui_task, (void*)&kvm_switch_task_ctrl);
    thread_init(&t2, kvm_switch_task, (void*)&kvm_switch_task_ctrl);
    scheduler_register_thread(&scheduler, &t1);
    scheduler_register_thread(&scheduler, &t2);

    sei();

    scheduler_run(&scheduler);
}