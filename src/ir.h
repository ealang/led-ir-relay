#ifndef IR_H_
#define IR_H_

#include <stdint.h>

#define IR_BUFFER_SIZE 0x80

typedef struct {
    uint16_t buffer[IR_BUFFER_SIZE];
    uint8_t length;
    uint32_t last_time;
} IRBuffer;

void ir_buffer_init(IRBuffer *ir_buffer);
void ir_buffer_set_global_inst(IRBuffer *ir_buffer);

void ir_buffer_clear(void);
uint16_t *ir_buffer_contents(void);
uint8_t ir_buffer_length(void);

void ir_init_hardware(void);

// PCINT0_vect ISR
void ir_receive_handler(void);

typedef enum { IRPort0 = 0, IRPort1 = 1 } IRPort;

// Playback IR recording on the given port
void ir_play(IRPort port, uint16_t *buffer, uint8_t length);

#endif