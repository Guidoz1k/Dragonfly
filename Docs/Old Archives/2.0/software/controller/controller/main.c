#include <avr/io.h>
#include <avr/interrupt.h>

#define byte		uint8_t
#define twobyte	uint16_t
#define fourbyte	uint32_t
#define TMR 500

ISR(TIMER0_COMPA_vect){
	static twobyte counter = 0;

	if(counter < TMR)
		counter++;
	else{
		counter = 0;
		PINB = 0x20;	// toggle the led output status
	}
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



/// DISPLAY HW //////////////////////////////////////////////////////////////// *** DISPLAY FUNCTIONS ***
/*
	Display:
		PORTD 2, 3, 4, 5, 6, 7	==> DISPLAY data 2, 3, 4, 5, 6, 7
		PORTB 0, 2, 3, 4			==> DISPLAY E, data 0, 1 and RS

	the following are the hardware level functions, meant to operate
	only in the 1ms interrupt
*/

void lcdData(byte data){
	PORTB &= 0x02;							// (PORTB - BUZZER) = 0
	PORTD = 0x00;
	PORTD |= data & 0xFC;				// bits 7 - 2
	PORTB |= (data & 0x03) << 0x02;	// bit 0 and 1
}

void lcdEnablePulse(){
	PORTB &= 0xFE;						// E = 0
	PORTB |= 0x01;						// E = 1;
	nopi();								// 1us min delay
	PORTB &= 0xFE;						// E = 0;
	for(byte i = 0; i < 40; i++)	// 40us min delay
		nopi();
	}

void lcdConfig(byte data){
	lcdData(data);		//(including RS = 0)
	nopi();
	lcdEnablePulse();
	}

void lcdFlush(){
	PORTD = 0x00;
	PORTB &= 0x02;		// (PORTB - BUZZER) = 0 (including RS = 0)
	PORTB |= 0x05;		// data = 0x01 + E = 1;
	nopi();				// 1us min delay
	PORTB &= 0xFE;		// E = 0;

	/*for(twobyte i = 0; i < 1650; i++)	// 1.65ms min delay
		nopi();*/ // now its executed in displayMain()
	}

void lcdWrite(byte data){
	lcdData(data);			//(including RS = 0)
	PORTB |= 0x10;			// RS = 1;
	lcdEnablePulse();
	}

void lcdPos(byte line, byte pos){
	if(line)				// display second line
		pos |= 0x40;	// pos 0 of second line is memory position 0x40
	pos |= 0x80;		// config bit set
	lcdConfig(pos);
	}

void displayInit(){
	lcdConfig(0x06);	// display automatic cursor increment
	lcdConfig(0x0c);	// active display with hidden cursor
	lcdConfig(0x38);	//	bit and pixel format
	lcdFlush();
	}

/// DISPLAY LL ////////////////////////////////////////////////////////////////
/*
	the following are the low level functions, meant to operate
	only when called by the scheduler
*/

byte displayBuffer[19];
/*
	display writing buffer 16 positions + '\0' + 2 positioning characters

	if character is 0 or 1:
		- the first character is the line
		- the second character is the position within the line
	otherwise is simple string
*/
byte displayWriting = 0;	// display is writing
byte displayFlush = 0;		// display is flushing
byte displayBlocked = 0;		// display block flag

void displaySetup(byte status){
	if(status)
		displayBlocked = 0;	// blocks display buffer operations
	else{
		displayBlocked = 1;	// unlocks the display and clear the writing process
		displayWriting = 0;
		}
	}

void displayWrite(){
	if(displayWriting || !displayBlocked){									// busy status and blocked status check
		if( (displayBuffer[0] == 0) || (displayBuffer[0] == 1) ){	// special character check
			lcdPos(displayBuffer[0], displayBuffer[1]);
			for(byte i = 0; i < 16; i++)										// remove an extra character from the buffer
				displayBuffer[i] = displayBuffer[i + 1];					//-so the reposition only wastes one cycle
			}
		else
			lcdWrite(displayBuffer[0]);					// not a special character

		for(byte i = 0; i < 16; i++)						// standart buffer shift
			displayBuffer[i] = displayBuffer[i + 1];
		}

	if(displayBuffer[0] == '\0')	// end of buffer
		displayWriting = 0;
	}

void displayMain(){
/*
	the main function of this main is to control wether
	there is a flush going on or not. in the case of flush
	the program should not operate the display until the
	flush has ended.
*/

	static byte flushCount = 0;	// delay counter for the flush operation
	byte flow = 0;						// flow of operation

	if(displayFlush)					// verify ongoing flush not waiting for writing to end
		flow = 2;
	else
		if(displayWriting)			// verify ongoing writing
			flow = 1;

	switch(flow){
		case 0:							// nothing is buffered
			break;
		case 1:							// execute writing
			displayWrite();
			break;
		case 2:							// execute the flow of flush
			if(!flushCount)			// flush is starting
				lcdFlush();
			flushCount++;
			if(flushCount == 2){		// flush has ended (2 cycles of 4ms each)
				displayFlush = 0;
				flushCount = 0;
				}
			break;
		}
	}

