#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "timers.h"

void interruptInit(void){
	sei();			// global interrupts enable
	TCCR0A = 2;		// timer0 CTC mode
	TCCR0B = 3;		// timer0 prescaler 64
	OCR0A = 249;	// 249 in hex
	TIMSK0 = 2;		// timer0 compareA interrupt enable
}

void delayInit(void){
	TCCR1A = 0;
	TCCR1B = 0b00001000;

	TIFR1 = 2;
	TCNT1 = 0;
	}

void delayus(fourbyte delayValue){
	if(delayValue < MIN_US)
		delayValue = MIN_US;
	if(delayValue > MAX_US)
		delayValue = MAX_US;

	OCR1A = (delayValue << 1) - 8;	// - 1 - 7(static error)

	TCCR1B = 0b00001010;
	while(!(TIFR1 & 2));
	TCCR1B = 0b00001000;

	TIFR1 = 2;
	TCNT1 = 0;
	}

void delayms(fourbyte delayValue){
	if(delayValue == 0)
		delayValue = MIN_MS;
	if(delayValue > MAX_MS)
		delayValue = MAX_MS;

	OCR1A = (delayValue * 250) - 1;

	TCCR1B = 0b00001011;
	while(!(TIFR1 & 2)){}
	TCCR1B = 0b00001000;

	TIFR1 = 2;
	TCNT1 = 0;
	}

