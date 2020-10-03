#ifndef LED_H_
#define LED_H_

typedef enum { Led0 = 0, Led1 = 1, Led2 = 2 } LedNumber;

void led_init_hardware(void);

// Write LED state.
// @param num led to change
// @param state 0 for off, 1 for on, -1 for toggle
void led_set_state(LedNumber num, char state);

#endif