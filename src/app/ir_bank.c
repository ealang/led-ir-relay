#include "ir_bank.h"
#include <avr/eeprom.h>
#include <assert.h>

#define IR_BANK_PER_BANK_SIZE 0x100
#define IR_BANK_MAX_DATA_SIZE (IR_BANK_PER_BANK_SIZE - 2)

// Location for storage of data length info
static uint16_t *ir_bank_data_length_addr(uint8_t bank_num)
{
    return (uint16_t*)(IR_BANK_PER_BANK_SIZE * bank_num);
}

uint16_t *ir_bank_data(uint8_t bank_num)
{
    return ir_bank_data_length_addr(bank_num) + 1;
}

void ir_bank_save(uint8_t bank_num, const uint16_t *data, uint8_t length)
{
    assert(length <= IR_BANK_MAX_DATA_SIZE);
    eeprom_update_word(ir_bank_data_length_addr(bank_num), length);
    eeprom_update_block(
        (void*)data,
        (void*)ir_bank_data(bank_num),
        length * sizeof(uint16_t)
    );
}

// Get amount of IR data stored in EEPROM.
uint8_t ir_bank_data_length(uint8_t bank_num)
{
    return (uint8_t)eeprom_read_word(ir_bank_data_length_addr(bank_num));
}
