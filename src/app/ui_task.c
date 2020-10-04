#include <stdint.h>

#include "led_anim/led_anim.h"
#include "os/input.h"
#include "os/ir.h"

void ui_task(void *param);
void ui_task(void *param)
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