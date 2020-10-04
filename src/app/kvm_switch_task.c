#include "kvm_switch_task.h"

#include "bin_rolling_avg.h"
#include "ir_bank.h"

#include "led_anim/led_anim.h"

#include "os/ir.h"
#include "os/light_sensor.h"
#include "os/time.h"

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

static uint8_t ir_bank_lookup(IRPort ir_port, uint8_t active_kvm_port)
{
    if (ir_port == IRPort0)
    {
        return active_kvm_port;
    }
    return 2 + active_kvm_port;
}

static void ir_replay_one_port(IRPort ir_port, uint8_t active_kvm_port)
{
    uint8_t bank_num = ir_bank_lookup(ir_port, active_kvm_port);
    ir_play(
        ir_port,
        ir_bank_data(bank_num),
        ir_bank_data_length(bank_num),
        ir_bank_read
    );
}

// Play IR signal to reflect new active kvm port
static void ir_replay_all_ports(uint8_t active_kvm_port)
{
    ir_replay_one_port(IRPort0, active_kvm_port);
    ir_replay_one_port(IRPort1, active_kvm_port);
}

///////////////////////////////////////////////////////////////////////////////////

// Force mode port has changed
static void force_mode_loop__on_port_change(const State *state, uint8_t active_kvm_port)
{
    LedNumber led_number = active_kvm_port == 0 ? port0_led : port1_led;
    reset_leds(state);
    led_anim_play(led_number, state->led_anim_pulse, LED_ANIM_MAX_PROGRAM_LEN);

    ir_replay_all_ports(active_kvm_port);
}

// Main loop for force mode
static void force_mode_loop(const State *state)
{
    char initialized = 0;
    char last_active_port = 0;

    while (state->ctrl->port_selection != KVM_PORT_SELECTION_AUTO)
    {
        if (!initialized || state->ctrl->port_selection != last_active_port)
        {
            initialized = 1;
            last_active_port = state->ctrl->port_selection;
            force_mode_loop__on_port_change(state, state->ctrl->port_selection);
        }
        await_sleep(MS_TO_TICKS(100));
    }
}

///////////////////////////////////////////////////////////////////////////////////

// Auto mode port has changed
static void auto_mode_loop__on_port_change(const State *state, uint8_t active_kvm_port)
{
    LedNumber led_number = active_kvm_port == 0 ? port0_led : port1_led;
    reset_leds(state);
    led_anim_play(led_number, state->led_anim_on, LED_ANIM_MAX_PROGRAM_LEN);

    ir_replay_all_ports(active_kvm_port);
}

// Main loop for auto mode
// Poll KVM switch waiting for stable state change.
static void auto_mode_loop(const State *state)
{
    char initialized = 0;
    char last_active_port = 0;

    BinaryAverage sensor_history = {0, 0, 0};
    while (state->ctrl->port_selection == KVM_PORT_SELECTION_AUTO)
    {
        uint8_t cur_active_port = compute_rolling_avg(read_max_light_sensor(), &sensor_history);
        if (!initialized || cur_active_port != last_active_port)
        {
            initialized = 1;
            last_active_port = cur_active_port;
            auto_mode_loop__on_port_change(state, cur_active_port);
        }
        await_sleep(MS_TO_TICKS(500 / 16));
    }
}

///////////////////////////////////////////////////////////////////////////////////

// Task to monitor auto/force mode and issue IR commands.
// This thread relies on polling for KVM state and command updates.
// TODO: implement better thread comm mechanisms.
void kvm_switch_task(void *param)
{
    State state = { .ctrl = (KvmSwitchTaskCtrl*)param };
    led_anim_sequence_steady(state.led_anim_off, 0);
    led_anim_sequence_steady(state.led_anim_on, 1);
    led_anim_sequence_infinite_blink(state.led_anim_pulse, MS_TO_ANIM_TICKS(250));

    while(1)
    {
        if (state.ctrl->port_selection == KVM_PORT_SELECTION_AUTO)
        {
            auto_mode_loop(&state);
        }
        else
        {
            force_mode_loop(&state);
        }
    }
}
