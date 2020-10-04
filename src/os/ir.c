#include "ir.h"
#include "time.h"

#include <avr/io.h>

static IRBuffer *global_inst = 0;

#define IR_RECEIVE_PIN_NUM 0
#define IR_RECEIVE_PIN_MASK (1 << IR_RECEIVE_PIN_NUM)

#define IR_MIRROR_PIN_NUM 1
#define IR_MIRROR_PIN_MASK 0x2

#define IR_PLAY_PIN_0_NUM 2
#define IR_PLAY_PIN_0_MASK (1 << IR_PLAY_PIN_0_NUM)
#define IR_PLAY_PIN_1_NUM 3
#define IR_PLAY_PIN_1_MASK (1 << IR_PLAY_PIN_1_NUM)

// Put a reasonable time limit on how long playback sequences can be
#define IR_PLAY_MAX_TICKS MS_TO_TICKS(500)

void ir_buffer_init(IRBuffer *ir_buffer)
{
    ir_buffer->length = 0;
    ir_buffer->last_time = 0;
}

void ir_buffer_set_global_inst(IRBuffer *ir_buffer)
{
    global_inst = ir_buffer;
}

void ir_init_hardware(void)
{
    // Input pin
    DDRB &= ~IR_RECEIVE_PIN_MASK;  // Set as input
    PCMSK0 |= IR_RECEIVE_PIN_MASK;  // Enable interrupt on pin
    PCICR |= (1 << PCIE0);  // Enable interrupt on bank

    // Mirror and play pins
    uint8_t output_mask = IR_MIRROR_PIN_MASK | IR_PLAY_PIN_0_MASK | IR_PLAY_PIN_1_MASK;
    DDRB |= output_mask;  // Set as output
    PORTB &= ~output_mask;  // Set low
}

static inline uint16_t max(uint16_t a, uint16_t b)
{
    if (a >= b)
    {
        return a;
    }
    return b;
}

void ir_receive_handler(void)
{
    // Read IR bit
    uint8_t val = ((PINB >> IR_RECEIVE_PIN_NUM) & 1) ^ 1;

    // Mirror bit
    PORTB = (PORTB & ~IR_MIRROR_PIN_MASK) | (val << IR_MIRROR_PIN_NUM);

    if (!global_inst)
    {
        return;
    }

    uint16_t i = global_inst->length;
    if (
        i >= IR_BUFFER_SIZE ||  // Buffer is full
        (i == 0 && val == 0)  // Wait for high transition to start capture
    )
    {
        return;
    }

    ++global_inst->length;

    uint32_t time = system_time_ticks();
    global_inst->buffer[i] = max(time - global_inst->last_time, 1);
    global_inst->last_time = time;
}

void ir_buffer_clear(void)
{
    ir_buffer_init(global_inst);
}

uint16_t *ir_buffer_contents(void)
{
    return global_inst->buffer;
}

uint8_t ir_buffer_length(void)
{
    return global_inst->length;
}

static inline void ir_set_bit(IRPort port, uint8_t val)
{
    uint8_t num = port == IRPort0 ? IR_PLAY_PIN_0_NUM : IR_PLAY_PIN_1_NUM;
    uint8_t mask = 1 << num;
    PORTB = (PORTB & ~mask) | (val << num);
}

void ir_play(IRPort port, uint16_t *buffer, uint8_t length, uint16_t (*read_word)(const uint16_t*))
{
    const uint16_t final_ticks = MS_TO_TICKS(10);
    uint32_t total_ticks = 0;

    uint8_t val = 1;
    for (uint8_t i = 0; i < length; i++)
    {
        uint16_t ticks = (i == length - 1) ?
            final_ticks :
            read_word(buffer + i + 1);
        total_ticks += ticks;
        if (total_ticks > IR_PLAY_MAX_TICKS)
        {
            break;
        }

        ir_set_bit(port, val);
        await_sleep(ticks);
        val ^= 1;
    }
    ir_set_bit(port, 0);
}
