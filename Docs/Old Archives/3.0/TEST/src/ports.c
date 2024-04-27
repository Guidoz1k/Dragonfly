#include <avr/io.h>
#include <avr/interrupt.h>
#include "ports.h"

void writeB(uint8_t data){
	PORTB = data;
}

void writeC(uint8_t data){
	PORTC = data;
}

void writeD(uint8_t data){
	PORTD = data;
}

void bitWriteB(uint8_t bit, uint8_t data){
	if(data)
		PORTB |= 1 << bit;
	else
		PORTB &= ~(1 << bit);
}

void bitWriteC(uint8_t bit, uint8_t data){
	if(data)
		PORTC |= 1 << bit;
	else
		PORTC &= ~(1 << bit);
}

void bitWriteD(uint8_t bit, uint8_t data){
	if(data)
		PORTD |= 1 << bit;
	else
		PORTD &= ~(1 << bit);
}

uint8_t readB(void){
	return PINB;
}

uint8_t readC(void){
	return PINC;
}

uint8_t readD(void){
	return PIND;
}

uint8_t bitReadB(uint8_t bit){
	uint8_t data = 0;

	if(PINB & (1 << bit))
		data = 1;

	return data;
}

uint8_t bitReadC(uint8_t bit){
	uint8_t data = 0;

	if(PINC & (1 << bit))
		data = 1;

	return data;
}

uint8_t bitReadD(uint8_t bit){
	uint8_t data = 0;

	if(PIND & (1 << bit))
		data = 1;

	return data;
}

void directB(uint8_t data){
	DDRB = data;
}

void directC(uint8_t data){
	DDRC = data;
}

void directD(uint8_t data){
	DDRD = data;
}

void bitDirectB(uint8_t bit, uint8_t data){
	if(data)
		DDRB |= 1 << bit;
	else
		DDRB &= ~(1 << bit);
}

void bitDirectC(uint8_t bit, uint8_t data){
	if(data)
		DDRC |= 1 << bit;
	else
		DDRC &= ~(1 << bit);
}

void bitDirectD(uint8_t bit, uint8_t data){
	if(data)
		DDRD |= 1 << bit;
	else
		DDRD &= ~(1 << bit);
}

void toggleB(uint8_t data){
	PINB = data;
}

void toggleC(uint8_t data){
	PINC = data;
}

void toggleD(uint8_t data){
	PIND = data;
}

void bitToggleB(uint8_t bit){
	PINB = 1 << bit;
}

void bitToggleC(uint8_t bit){
	PINC = 1 << bit;
}

void bitToggleD(uint8_t bit){
	PIND = 1 << bit;
}
