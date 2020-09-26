#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

ISR(PCINT0_vect)
{
	// TODO: check which pin got set (?)
	PORTC = (PORTC & ~2) | (((~PINB) & 1) << 1);
}

/*

PC0 blink
PC1 mirrored output
PB0/PCINT0 input
*/
int main(void)
{
	DDRC = 0x3;
	PORTC = 1;
	
	DDRB = 0;
	PCMSK0 |= 1;
	PCICR |= 1;

	sei();
	while(1)
	{
		PORTC ^= 1;
		_delay_ms(1000);
	}
}