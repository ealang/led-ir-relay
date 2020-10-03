#include "led_anim.h"

#include <string.h>
#include <assert.h>

static LedAnimManager *global_inst = 0;

#define TICK_SCALING 6
#define UPDATE_INTERVAL_TICKS (1 << LED_ANIM_SYSTEM_TICKS_SCALE)
#define SINGLE_STEP_RESULT_NO_CHANGE 2

uint8_t led_anim_sequence_steady(LedOpCode *dest, char value)
{
    const LedOpCode const *start = dest;
    *(dest++) = led_opcode_change_state(value, 0);
    *(dest++) = led_opcode_halt();
    return (uint8_t)(dest - start);
}

uint8_t led_anim_sequence_infinite_blink(LedOpCode *dest, uint8_t ticks_per_state)
{
    const LedOpCode const *start = dest;
    *(dest++) = led_opcode_change_state(-1, ticks_per_state);
    *(dest++) = led_opcode_jump(0, 0);
    return (uint8_t)(dest - start);
}

uint8_t led_anim_sequence_pulse_then_off(LedOpCode *dest, uint8_t n_pulses, uint8_t ticks_per_state)
{
    const LedOpCode const *start = dest;
    *(dest++) = led_opcode_change_state(1, ticks_per_state);
    *(dest++) = led_opcode_change_state(0, ticks_per_state);
    if (n_pulses > 1)
    {
        *(dest++) = led_opcode_jump(0, n_pulses - 1);
    }
    *(dest++) = led_opcode_halt();
    return (uint8_t)(dest - start);
}

uint8_t led_anim_sequence_infinite_pulse(LedOpCode *dest, uint8_t n_pulses, uint8_t pulse_ticks, uint8_t pause_ticks)
{
    const LedOpCode const *start = dest;
    *(dest++) = led_opcode_change_state(1, pulse_ticks);
    *(dest++) = led_opcode_change_state(0, pulse_ticks);
    if (n_pulses > 1)
    {
        *(dest++) = led_opcode_jump(0, n_pulses - 1);
    }
    *(dest++) = led_opcode_change_state(0, pause_ticks);
    *(dest++) = led_opcode_jump(0, 0);
    return (uint8_t)(dest - start);
}

void led_anim_init(LedAnimManager *inst)
{
    for (uint8_t i = 0; i < LED_ANIM_NUM_LEDS; i++)
    {
        LedAnimData *data = &inst->leds[i];

        // Set a default no-op program
        data->program[0] = led_opcode_halt();
        data->pc = 0;
        data->next_update_countdown = 0;
    }
}

void led_anim_set_global_inst(LedAnimManager *inst)
{
    global_inst = inst;
}

// Run next instruction.
// @return led state to set: 0=off, 1=on, -1=toggle, 2=no change
static char single_step_animation(LedAnimData *data)
{
    if (data->next_update_countdown)
    {
        --data->next_update_countdown;
        return SINGLE_STEP_RESULT_NO_CHANGE;
    }

    LedOpCode *opcode = &data->program[data->pc];
    if (opcode->type == LedOpCodeHalt)
    {
        return SINGLE_STEP_RESULT_NO_CHANGE;
    }
    if (opcode->type == LedOpCodeChangeState)
    {
        ++data->pc;
        data->next_update_countdown = opcode->data.change_led_state.duration;
        return opcode->data.change_led_state.led_state;
    }
    if (opcode->type == LedOpCodeJump)
    {
        if (opcode->data.jump.exec_count == 0 || opcode->data.jump.remaining_exec > 0)
        {
            if (opcode->data.jump.remaining_exec)
            {
                --opcode->data.jump.remaining_exec;
            }
            data->pc = opcode->data.jump.location;
        }
        else
        {
            opcode->data.jump.remaining_exec = opcode->data.jump.exec_count;
            ++data->pc;
        }

        return single_step_animation(data);
    }
    assert(0);

    return SINGLE_STEP_RESULT_NO_CHANGE;
}

void led_anim_thread(void *param)
{
    LedAnimManager *anim = (LedAnimManager*)param;
    while(1)
    {
        for (uint8_t led = 0; led < LED_ANIM_NUM_LEDS; led++)
        {
            char new_state = single_step_animation(&anim->leds[led]);
            if (new_state != SINGLE_STEP_RESULT_NO_CHANGE)
            {
                led_set_state((LedNumber)led, new_state);
            }
        }
        await_sleep(UPDATE_INTERVAL_TICKS);
    }
}

void led_anim_play(LedNumber led, const LedOpCode *program, uint8_t program_length)
{
    if (!global_inst)
    {
        return;
    }
    assert (program_length <= LED_ANIM_MAX_PROGRAM_LEN);

    LedAnimData *data = &global_inst->leds[(uint8_t)led];
    memcpy(data->program, program, program_length * sizeof(LedOpCode));
    data->pc = 0;
    data->next_update_countdown = 0;
}