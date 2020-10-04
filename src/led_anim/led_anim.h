#ifndef LED_ANIM_H_
#define LED_ANIM_H_

#include "led_opcode.h"
#include "os/time.h"
#include "os/led.h"

#define LED_ANIM_NUM_LEDS 3
#define LED_ANIM_MAX_PROGRAM_LEN 10
#define LED_ANIM_SYSTEM_TICKS_SCALE 6

// Animation data for a single LED.
typedef struct
{
    LedOpCode program[LED_ANIM_MAX_PROGRAM_LEN];
    uint8_t pc;
    uint8_t next_update_countdown;
} LedAnimData;

// Animation data for all LEDs.
typedef struct
{
    LedAnimData leds[LED_ANIM_NUM_LEDS];
} LedAnimManager;

void led_anim_init(LedAnimManager *inst);
void led_anim_set_global_inst(LedAnimManager *inst);

// Thread to execute led animations.
void led_anim_thread(void *led_anim_manager);

// Start playing the given animation sequence on the given led.
void led_anim_play(LedNumber led, const LedOpCode *program, uint8_t program_length);

// Convert milliseconds to animation manager ticks
#define MS_TO_ANIM_TICKS(x) (MS_TO_TICKS(x) >> LED_ANIM_SYSTEM_TICKS_SCALE)

// Steady value "sequence".
// @param dest Array to write opcodes to
// @param value 0 for off, 1 for on
// @return length of sequence
uint8_t led_anim_sequence_steady(LedOpCode *dest, char value);

// Infinite blink sequence.
// @param dest Array to write opcodes to
// @param ticks_per_state Determine blink rate
// @return length of sequence
uint8_t led_anim_sequence_infinite_blink(LedOpCode *dest, uint8_t ticks_per_state);

#endif
