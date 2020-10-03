#include "led.h"

#include <avr/io.h>

#define LED0_MASK 1
#define LED1_MASK 2
#define LED2_MASK 4
#define LED_ALL_MASK (LED0_MASK | LED1_MASK | LED2_MASK)

void led_init_hardware(void)
{
    DDRC |= LED_ALL_MASK;
    PORTC &= ~LED_ALL_MASK;
}

void led_set_state(LedNumber num, char state)
{
    uint8_t mask = LED0_MASK;
    if (num == Led1)
    {
        mask = LED1_MASK;
    }
    else if (num == Led2)
    {
        mask = LED2_MASK;
    }

    if (state == 0)
    {
        PORTC &= ~mask;
    }
    else if (state == 1)
    {
        PORTC |= mask;
    }
    else
    {
        PORTC ^= mask;
    }
}