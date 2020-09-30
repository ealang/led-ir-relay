#include "input.h"
#include "scheduler.h"
#include "time.h"

#include <avr/io.h>
#include <assert.h>

#define INPUT_PIN_DOWN_VAL 0
#define INPUT_PIN_UP_VAL 1
#define INPUT_PIN_MASK 0x3

#define DEBOUNCE_TIME_TICKS ((int)(0.01 / SEC_PER_CLOCK_TICK))
#define SHORT_PRESS_TICKS ((int)(0.7 / SEC_PER_CLOCK_TICK))
#define LONG_PRESS_TICKS ((int)(10 / SEC_PER_CLOCK_TICK))

static InputManager *global_inst = 0;

void input_manager_init_hardware(void)
{
    PORTD |= INPUT_PIN_MASK;  // pull-up resistors
    DDRD &= ~INPUT_PIN_MASK;  // input pins

    PCMSK2 |= INPUT_PIN_MASK;  // enable interrupts on pins
    PCICR |= (1 << PCIE2);  // enable interrupt bank 2
}

static uint8_t input_manager_read_button_states(void)
{
    return PIND & INPUT_PIN_MASK;
}

static uint16_t clock_tick_delta(uint16_t time1, uint16_t time2)
{
    if (time2 >= time1)
    {
        return time2 - time1;
    }

    // Wraped around
    return ((uint16_t)-1) - time1 + time2 + 1;
}

// Given a recognized button press, notify any subscribers.
static void on_button_press(InputManager *inst, uint8_t button_num, Gesture gesture)
{
    for (uint8_t i = 0; i < MAX_INPUT_SUBSCRIBERS; ++i)
    {
        Pipe *pipe = inst->subscriber_pipe[i];
        if (pipe && inst->subscriber_button_num[i] == button_num)
        {
            pipe->ready_bool = 1;
            pipe->value.as_pair.byte1 = gesture;
            inst->subscriber_pipe[i] = 0;
        }
    }
}

// Got a input change interrupt.
void input_manager_pin_change_handler(void)
{
    if (!global_inst)
    {
        return;
    }

    uint8_t button_states = input_manager_read_button_states();
    uint8_t change_mask = button_states ^ global_inst->prev_state;
    global_inst->prev_state = button_states;

    uint16_t cur_time = system_time_ticks();
    for (uint8_t button_num = 0; button_num < NUM_BUTTONS; ++button_num)
    {
        if (change_mask & 1)
        {
            uint8_t val = button_states & 1;
            if (val == INPUT_PIN_DOWN_VAL)
            {
                global_inst->button_down_time[button_num] = cur_time;
            }
            else
            {
                uint16_t prev_time = global_inst->button_down_time[button_num];
                uint16_t passed_time = clock_tick_delta(prev_time, cur_time);
                if (passed_time >= DEBOUNCE_TIME_TICKS)
                {
                    if (passed_time <= SHORT_PRESS_TICKS)
                    {
                        on_button_press(global_inst, button_num, ShortPress);
                    }
                    else if (passed_time <= LONG_PRESS_TICKS)
                    {
                        on_button_press(global_inst, button_num, LongPress);
                    }
                }
            }
        }
        change_mask >>= 1;
        button_states >>= 1;
    }
}

void input_manager_init(InputManager *inst)
{
    for (uint8_t i = 0; i < MAX_INPUT_SUBSCRIBERS; ++i)
    {
        inst->subscriber_pipe[i] = 0;
        inst->subscriber_button_num[i] = 0;
    }

    inst->prev_state = INPUT_PIN_MASK * INPUT_PIN_UP_VAL; // assume starts "up"
}

void input_manager_set_global_inst(InputManager *inst)
{
    global_inst = inst;
}

static uint8_t get_free_subscriber_slot(InputManager *inst)
{
    for (uint8_t i = 0; i < MAX_INPUT_SUBSCRIBERS; ++i)
    {
        if (inst->subscriber_pipe[i] == 0)
        {
            return i;
        }
    }
    return -1;
}

Gesture await_input(ButtonNumber button_num)
{
    Pipe pipe;
    pipe_init(&pipe);

    uint8_t slot = get_free_subscriber_slot(global_inst);
    assert (slot != -1);
    global_inst->subscriber_button_num[slot] = button_num;
    global_inst->subscriber_pipe[slot] = &pipe;  // Do this last in case ISR runs

    scheduler_await(&pipe);

    return (Gesture)pipe.value.as_pair.byte1;
}
