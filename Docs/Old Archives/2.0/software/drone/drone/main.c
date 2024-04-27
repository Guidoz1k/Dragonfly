#include <avr/io.h>
#include <avr/interrupt.h>

#define byte		uint8_t
#define twobyte	uint16_t
#define fourbyte	uint32_t
#define TMR 500

void scheduler(void){

}

ISR(TIMER0_COMPA_vect){
	static twobyte counter = 0;

	if(counter < TMR)
		counter++;
	else{
		counter = 0;
		PINB = 0x20;	// toggle the led output status
	}
	scheduler();
}

int main(void){
	DDRB = 0xFF;		// PORTB is output
	sei();				// global interrupts enable
	TCCR0A = 0x02;		// timer0 CTC mode
	TCCR0B = 0x03;		// timer0 prescaler 64
	OCR0A = 0xF9;		// 249 in hex
	TIMSK0 = 0x02;		// timer0 compareA interrupt enable

   while(1){
   }
}
