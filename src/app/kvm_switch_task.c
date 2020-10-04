#include "kvm_switch_task.h"
#include "bin_rolling_avg.h"

#include "led_anim/led_anim.h"
#include "os/time.h"
#include "os/light_sensor.h"

#include <stdint.h>

static const LedNumber port0_led = Led0;
static const LedNumber port1_led = Led1;

typedef struct {
    volatile KvmSwitchTaskCtrl *ctrl;
    LedOpCode led_anim_off[LED_ANIM_MAX_PROGRAM_LEN];
    LedOpCode led_anim_on[LED_ANIM_MAX_PROGRAM_LEN];
    LedOpCode led_anim_pulse[LED_ANIM_MAX_PROGRAM_LEN];
} State;

static void reset_leds(const State *state)
{
    led_anim_play(port0_led, state->led_anim_off, LED_ANIM_MAX_PROGRAM_LEN);
    led_anim_play(port1_led, state->led_anim_off, LED_ANIM_MAX_PROGRAM_LEN);
}

///////////////////////////////////////////////////////////////////////////////////

// Force mode port has changed
static void force_mode_loop__on_port_change(const State *state, uint8_t new_port)
{
    LedNumber led_number = new_port == 0 ? port0_led : port1_led;
    reset_leds(state);
    led_anim_play(led_number, state->led_anim_pulse, LED_ANIM_MAX_PROGRAM_LEN);
    // TODO: replay IR
}

// Main loop for force mode
static void force_mode_loop(const State *state)
{
    char initialized = 0;
    char last_port = 0;

    while (state->ctrl->port_selection != KVM_SWITCH_PORT_AUTO)
    {
        if (!initialized || state->ctrl->port_selection != last_port)
        {
            initialized = 1;
            last_port = state->ctrl->port_selection;
            force_mode_loop__on_port_change(state, state->ctrl->port_selection);
        }
        await_sleep(MS_TO_TICKS(100));
    }
}

///////////////////////////////////////////////////////////////////////////////////

// Auto mode port has changed
static void auto_mode_loop__on_port_change(const State *state, uint8_t new_port)
{
    LedNumber led_number = new_port == 0 ? port0_led : port1_led;
    reset_leds(state);
    led_anim_play(led_number, state->led_anim_on, LED_ANIM_MAX_PROGRAM_LEN);
    // TODO: replay IR
}

// Main loop for auto mode
static void auto_mode_loop(const State *state)
{
    char initialized = 0;
    char last_port = 0;

    BinaryAverage sensor_history = {0, 0, 0};
    while (state->ctrl->port_selection == KVM_SWITCH_PORT_AUTO)
    {
        uint8_t cur_port = compute_rolling_avg(read_max_light_sensor(), &sensor_history);
        if (!initialized || cur_port != last_port)
        {
            initialized = 1;
            last_port = cur_port;
            auto_mode_loop__on_port_change(state, cur_port);
        }
        await_sleep(MS_TO_TICKS(100 / 16));
    }
}

///////////////////////////////////////////////////////////////////////////////////

// Task to monitor auto/force mode and issue commands to switches.
void kvm_switch_task(void *param)
{
    State state = { .ctrl = (KvmSwitchTaskCtrl*)param };
    led_anim_sequence_steady(state.led_anim_off, 0);
    led_anim_sequence_steady(state.led_anim_on, 1);
    led_anim_sequence_infinite_blink(state.led_anim_pulse, MS_TO_ANIM_TICKS(100));

    while(1)
    {
        if (state.ctrl->port_selection == KVM_SWITCH_PORT_AUTO)
        {
            auto_mode_loop(&state);
        }
        else
        {
            force_mode_loop(&state);
        }
    }
}
