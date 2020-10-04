#ifndef IR_BANK_H_
#define IR_BANK_H_

#include <stdint.h>
#include <avr/eeprom.h>

// Save data to EEPROM.
void ir_bank_save(uint8_t bank_num, const uint16_t *data, uint8_t length);

// Get address of IR data in EEPROM.
uint16_t *ir_bank_data(uint8_t bank_num);

// Get amount of IR data stored in EEPROM.
uint8_t ir_bank_data_length(uint8_t bank_num);

// Read data using eeprom.
#define ir_bank_read eeprom_read_word

#endif
