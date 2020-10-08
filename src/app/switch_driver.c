#include "switch_driver.h"
#include "ir_bank.h"

#include "led_anim/led_anim.h"

#include "os/ir.h"
#include "os/mutex.h"
#include "os/light_sensor.h"
#include "os/time.h"

#include <stdint.h>

static const LedNumber port0_led = Led0;
static const LedNumber port1_led = Led1;

///////////////////////////////////////////////////////////////////////////////////

static uint8_t ir_bank_lookup(IRPort ir_port, SwitchPort port_select)
{
    if (ir_port == IRPort0)
    {
        return (uint8_t)port_select;
    }
    return 2 + (uint8_t)port_select;
}

static void ir_replay_one_port(IRPort ir_port, SwitchPort port_select)
{
    uint8_t bank_num = ir_bank_lookup(ir_port, port_select);
    ir_play(
        ir_port,
        ir_bank_data(bank_num),
        ir_bank_data_length(bank_num),
        ir_bank_read
    );
}

// Play IR signal to reflect new active kvm port
static void ir_replay_all_ports(SwitchPort port_select)
{
    ir_replay_one_port(IRPort0, port_select);
    ir_replay_one_port(IRPort1, port_select);
}

///////////////////////////////////////////////////////////////////////////////////

static void set_led_animation_state(SwitchDriverState *state)
{
    LedOpCode inactive_anim[LED_ANIM_MAX_PROGRAM_LEN];
    led_anim_sequence_steady(inactive_anim, 0);

    LedOpCode active_anim[LED_ANIM_MAX_PROGRAM_LEN];
    if (state->mode == AutoSelect)
    {
        led_anim_sequence_steady(active_anim, 1);
    }
    else
    {
        led_anim_sequence_infinite_blink(active_anim, MS_TO_ANIM_TICKS(250));
    }

    LedNumber active_led = state->last_port == SwitchPort0 ? port0_led : port1_led;
    LedNumber inactive_led = state->last_port == SwitchPort0 ? port1_led : port0_led;
    led_anim_play(active_led, active_anim, LED_ANIM_MAX_PROGRAM_LEN);
    led_anim_play(inactive_led, inactive_anim, LED_ANIM_MAX_PROGRAM_LEN);
}

///////////////////////////////////////////////////////////////////////////////////

void switch_driver_state_init(SwitchDriverState *state, Mutex *mutex)
{
    state->mode = AutoSelect;
    state->mutex = mutex;
    state->last_port = SwitchPortNone;
    state->active_kvm_port = SwitchPortNone;
}

void switch_driver_set_mode_auto(SwitchDriverState *state)
{
    await_mutex_acquire(state->mutex);
    if (state->mode != AutoSelect)
    {
        state->mode = AutoSelect;

        // Do nothing if we don't know which port is active yet
        if (state->active_kvm_port != SwitchPortNone)
        {
            if (state->active_kvm_port != state->last_port)
            {
                state->last_port = state->active_kvm_port;
                ir_replay_all_ports(state->active_kvm_port);
            }
            set_led_animation_state(state);
        }
    }
    mutex_release(state->mutex);
}

void switch_driver_set_mode_force(SwitchDriverState *state, uint8_t port)
{
    await_mutex_acquire(state->mutex);
    if ((uint8_t)state->mode != port)
    {
        state->mode = port;
        if (port != state->last_port)
        {
            state->last_port = port;
            ir_replay_all_ports(port);
        }
        set_led_animation_state(state);
    }
    mutex_release(state->mutex);
}

uint8_t swtich_driver_get_active_port(const SwitchDriverState *state)
{
    return state->last_port;
}

void swtich_driver_report_active_kvm_port(SwitchDriverState *state, uint8_t port)
{
    await_mutex_acquire(state->mutex);
    if (state->active_kvm_port != port)
    {
        state->active_kvm_port = port;
        if (state->mode == AutoSelect && state->last_port != port)
        {
            state->last_port = port;
            ir_replay_all_ports(port);
        }
        set_led_animation_state(state);
    }
    mutex_release(state->mutex);
}
