#include "led_opcode.h"

LedOpCode led_opcode_change_state(char led_state, uint8_t ticks)
{
    LedOpCode code = {
        .type = LedOpCodeChangeState,
        .data = {
            .change_led_state = { led_state, ticks }
        }
    };
    return code;
}

LedOpCode led_opcode_jump(uint8_t location, char repeat_count)
{
    LedOpCode code = {
        .type = LedOpCodeJump,
        .data = {
            .jump = { location, repeat_count }
        }
    };
    return code;
}

LedOpCode led_opcode_halt(void)
{
    LedOpCode code = {
        .type = LedOpCodeHalt,
    };
    return code;
}