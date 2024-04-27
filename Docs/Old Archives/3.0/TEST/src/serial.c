#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"

void serialInit9600(void){
	UBRR0H = 0x00;		// baud rate of 9600 equals to UBRR 0x0067
	UBRR0L = 0x67;		//
	UCSR0B = 0x18;		// enables RX and TX
	UCSR0C = 0x06;		// set frame format: 8data
}

uint8_t serialReadAvailable(void){
	uint8_t data = 0;

	if(UCSR0A & 0x80)
		data = 1;

	return data;	// USART receive complete bit (RXC0) is 1
}

uint8_t serialRead(void){
	return UDR0;
}

uint8_t serialWriteAvailable(void){
	uint8_t data = 0;

	if(UCSR0A & 0x20)	// USART data register empty (UDRE0) is 1
		data = 1;

	return data;	
}

void serialWrite(uint8_t data){
	UDR0 = data;
}

void  serialWriteBinary(uint8_t data){
	for(int i = 8; i > 0; i--){
		while(!serialWriteAvailable());
		if(data & (1 << (i - 1)))
			serialWrite(49);
		else
			serialWrite(48);
	}
}

void serialWriteString(char *data){
	while(*data != '\0'){
		while(!serialWriteAvailable());
		serialWrite(*data);
		data++;
	}
}

void serialWriteNumber(uint32_t number){
	uint8_t counter = 8;
	uint8_t string[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	while(number != 0 && counter < 9){
		string[counter] = number % 10;
		number /= 10;
		counter--;
	}

	counter = 0;
	while(string[counter] == 0 && counter < 8)
		counter++;

	for(; counter < 9; counter++){
		while(!serialWriteAvailable());
		serialWrite(string[counter] + 48);
	}
}