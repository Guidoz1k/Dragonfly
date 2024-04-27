/*
	Buzzer ==> PORTB 1
	buzzTime = 16000000 / ( freq * 128)
	buzz freq:
		- 3500Hz: 35
		- 2500Hz: 50
*/
#include <avr/io.h>
#include <avr/interrupt.h>

#define byte		uint8_t
#define twobyte	uint16_t
#define fourbyte	uint32_t

void buzzInit(){
	sei();				// global interrupts enable
	TCCR1B = 0x0B;		// prescaler is 64
	}

void buzzOn(){
	TCCR1A = 0x40;		// timer 2 CTC mode, compare output mode Toggle OC1A
	}

void buzzOff(){
	TCCR1A = 0x80;		// timer 2 CTC mode, compare output mode Clear OC1A
	}

void buzzTime(byte high, byte low){
	TCNT1L = 0x00;		//	to avoid desync
	TCNT1L = 0x00;		//
	OCR1AH = high;
	OCR1AL = low;
	}