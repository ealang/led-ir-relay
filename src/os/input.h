#ifndef INPUT_H_
#define INPUT_H_

#include <stdint.h>

#define MAX_INPUT_SUBSCRIBERS 1
#define NUM_BUTTONS 1

typedef struct Pipe Pipe;

typedef struct 
{
    Pipe *subscriber_pipe[MAX_INPUT_SUBSCRIBERS];
    uint8_t subscriber_button_num[MAX_INPUT_SUBSCRIBERS];
    uint32_t button_down_time[NUM_BUTTONS];
    uint8_t prev_state;
} InputManager;

void input_manager_init_hardware(void);
void input_manager_pin_change_handler(void);
void input_manager_init(InputManager *inst);
void input_manager_set_global_inst(InputManager *inst);

typedef enum {ShortPress = 0, LongPress = 1} Gesture;
typedef enum {Button0 = 0} ButtonNumber;

// Block waiting for input. Thread aware.
Gesture await_input(ButtonNumber button_num);

#endif