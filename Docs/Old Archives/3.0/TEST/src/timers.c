#include <avr/io.h>
#include <avr/interrupt.h>

#include "timers.h"

void interruptInit(uint8_t prescaler, uint8_t compare){
    if(prescaler <= 5){
        sei();			        // global interrupts enable
        TCCR0A = 2;		        // timer0 CTC mode
        TCCR0B = 0;		        // timer0 prescaler = no clock
        OCR0A = compare - 1;    // output compare registerq
        TIMSK0 = 2;		        // timer0 compareA interrupt enable
        TCCR0B = prescaler;     // prescaler selection
    }
}

void delayInit(void){
	TCCR1A = 0;
	TCCR1B = 0b00001000;

	TIFR1 = 2;
	TCNT1 = 0;
	}

void delayus(uint16_t delayValue){
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

void delayms(uint16_t delayValue){
    uint16_t limit = delayValue / 250;
    uint16_t rest = delayValue % 250;
    uint16_t counter = 0;

    if(rest){
        OCR1A = (rest * 250) - 1;

        TCCR1B = 0b00001011;
        while(!(TIFR1 & 2)){}
        TCCR1B = 0b00001000;

        TIFR1 = 2;
        TCNT1 = 0;
        counter++;
    }

    for(; counter < limit; counter++){
        OCR1A = 62499;

        TCCR1B = 0b00001011;
        while(!(TIFR1 & 2)){}
        TCCR1B = 0b00001000;

        TIFR1 = 2;
        TCNT1 = 0;
    }
    

}

