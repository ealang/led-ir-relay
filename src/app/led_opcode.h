#ifndef LED_OPCODE_H_
#define LED_OPCODE_H_

#include <stdint.h>

typedef struct
{
    char led_state;
    uint8_t duration;
} LedOpCodeDataChangeState;

typedef struct
{
    uint8_t location;
    char repeat_count;
} LedOpCodeDataJump;

typedef enum { LedOpCodeChangeState, LedOpCodeJump, LedOpCodeHalt } LedOpCodeType;

typedef struct
{
    LedOpCodeType type;
    union {
        LedOpCodeDataChangeState change_led_state;
        LedOpCodeDataJump jump;
    } data;
} LedOpCode;

// Make an opcode to change on/off state of the LED.
// @param led_state 0 for off, 1 for on, -1 for toggle.
// @param ticks Number of animation ticks to hold for
LedOpCode led_opcode_change_state(char led_state, uint8_t ticks);

// Make an opcode to jump to another location.
// @param location Address/instruction number to jump to
// @param ticks How many times to perform the jump. -1 for infinite.
LedOpCode led_opcode_jump(uint8_t location, char repeat_count);

// Make an opcode to halt the program.
LedOpCode led_opcode_halt(void);

#endif /* LED_OPCODE_H_ */