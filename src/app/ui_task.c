#include "kvm_switch_task.h"
#include "ir_bank.h"

#include "led_anim/led_anim.h"
#include "led_anim/led_opcode.h"

#include "os/input.h"
#include "os/ir.h"
#include "os/time.h"

#include <stdint.h>

static const LedNumber status_led = Led2;

// LED animation: Pause, Pulse n times, repeat
static uint8_t led_anim_sequence_infinite_pulse(LedOpCode *dest, uint8_t n_pulses, uint8_t pulse_ticks, uint8_t pause_ticks)
{
    const LedOpCode const *start = dest;

    // Pause up front
    *(dest++) = led_opcode_change_state(0, pause_ticks);

    // Pulse & repeat section
    *(dest++) = led_opcode_change_state(1, pulse_ticks);
    *(dest++) = led_opcode_change_state(0, pulse_ticks);
    if (n_pulses > 1)
    {
        *(dest++) = led_opcode_jump(1, n_pulses - 1);
    }

    *(dest++) = led_opcode_jump(0, 0);
    return (uint8_t)(dest - start);
}

// UI to capture IR for a single bank
static void ui_task__ir_capture__bank(uint8_t bank_num)
{
    uint8_t anim_length;
    LedOpCode anim[LED_ANIM_MAX_PROGRAM_LEN];

    anim_length = led_anim_sequence_infinite_blink(anim, MS_TO_ANIM_TICKS(400));
    led_anim_play(status_led, anim, anim_length);

    ir_buffer_clear();

    Gesture press = await_input(Button0);
    if (press == LongPress)
    {
        uint8_t ir_buffer_len = ir_buffer_length();
        if (ir_buffer_len == IR_BUFFER_SIZE || ir_buffer_len == 0)
        {
            // Avoid saving if buffer is full (or empty), we are probably truncating the signal
            // TODO: Have a buffer overflow flag
            anim_length = led_anim_sequence_infinite_blink(anim, MS_TO_ANIM_TICKS(100));
            led_anim_play(status_led, anim, anim_length);
        }
        else
        {
            // Save IR data
            anim_length = led_anim_sequence_steady(anim, 0);
            led_anim_play(status_led, anim, anim_length);

            ir_bank_save(
                bank_num,
                ir_buffer_contents(),
                ir_buffer_len
            );
        }

        // Let LED animation play for a while
        await_sleep(MS_TO_TICKS(1000));
    }
}

// UI to select bank and capture IR
static void ui_task__ir_capture(void)
{
    LedOpCode anim_pulse[LED_ANIM_MAX_PROGRAM_LEN];

    uint8_t cur_bank = 0;
    while (1)
    {
        led_anim_sequence_infinite_pulse(anim_pulse, cur_bank + 1, MS_TO_ANIM_TICKS(100), MS_TO_ANIM_TICKS(800));
        led_anim_play(status_led, anim_pulse, LED_ANIM_MAX_PROGRAM_LEN);

        Gesture press = await_input(Button0);
        if (press == ShortPress)
        {
            if (++cur_bank > 3)
            {
                break;
            }
        }
        else
        {
            // Go into sub-menu for bank specific capture
            ui_task__ir_capture__bank(cur_bank);
            break;
        }
    }
}

// Cycle through ports/modes
static char next_port_selection(char cur_port)
{
    if (cur_port == 0)
    {
        return 1;
    }
    else if (cur_port == 1)
    {
        return KVM_PORT_SELECTION_AUTO;
    }
    else
    {
        return 0;
    }
}

void ui_task(void *param);
void ui_task(void *param)
{
    KvmSwitchTaskCtrl *ctrl = (KvmSwitchTaskCtrl*)param;

    LedOpCode anim_steady[LED_ANIM_MAX_PROGRAM_LEN];
    uint8_t anim_length = led_anim_sequence_steady(anim_steady, 1);

    while (1)
    {
        led_anim_play(status_led, anim_steady, anim_length);

        Gesture press = await_input(Button0);
        if (press == ShortPress)
        {
            // Signal kvm switch thread to change modes
            ctrl->port_selection = next_port_selection(ctrl->port_selection);
        }
        else
        {
            // Go into IR capture sub-menu
            ui_task__ir_capture();
        }
    }
}
