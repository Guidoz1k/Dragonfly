#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "ports.h"

void writeB(byte data){
	PORTB = data;
}

void writeC(byte data){
	PORTC = data;
}

void writeD(byte data){
	PORTD = data;
}

void bitWriteB(byte bit, byte data){
	if(data)
		PORTB |= 1 << bit;
	else
		PORTB &= ~(1 << bit);
}

void bitWriteC(byte bit, byte data){
	if(data)
		PORTC |= 1 << bit;
	else
		PORTC &= ~(1 << bit);
}

void bitWriteD(byte bit, byte data){
	if(data)
		PORTD |= 1 << bit;
	else
		PORTD &= ~(1 << bit);
}

byte readB(){
	return PINB;
}

byte readC(){
	return PINC;
}

byte readD(){
	return PIND;
}

byte bitReadB(byte bit){
	byte data = 0;

	if(PINB & (1 << bit))
		data = 1;

	return data;
}

byte bitReadC(byte bit){
	byte data = 0;

	if(PINC & (1 << bit))
		data = 1;

	return data;
}

byte bitReadD(byte bit){
	byte data = 0;

	if(PIND & (1 << bit))
		data = 1;

	return data;
}

void directB(byte data){
	DDRB = data;
}

void directC(byte data){
	DDRC = data;
}

void directD(byte data){
	DDRD = data;
}

void bitDirectB(byte bit, byte data){
	if(data)
		DDRB |= 1 << bit;
	else
		DDRB &= ~(1 << bit);
}

void bitDirectC(byte bit, byte data){
	if(data)
		DDRC |= 1 << bit;
	else
		DDRC &= ~(1 << bit);
}

void bitDirectD(byte bit, byte data){
	if(data)
		DDRD |= 1 << bit;
	else
		DDRD &= ~(1 << bit);
}

void toggleB(byte data){
	PINB = data;
}

void toggleC(byte data){
	PINC = data;
}

void toggleD(byte data){
	PIND = data;
}

void bitToggleB(byte bit){
	PINB = 1 << bit;
}

void bitToggleC(byte bit){
	PINC = 1 << bit;
}

void bitToggleD(byte bit){
	PIND = 1 << bit;
}
