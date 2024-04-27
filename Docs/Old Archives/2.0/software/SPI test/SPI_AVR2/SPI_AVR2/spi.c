#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "ports.h"
#include "spi.h"
#include "timers.h"

////////////////////////////////////////////////////////////// LEVEL 0 INSTRUCTIONS

byte spiTransfer(byte data){
	SPDR = data;
	while(!(SPSR & 0b10000000)){}
	return SPDR;
}

////////////////////////////////////////////////////////////// LEVEL 1 INSTRUCTIONS

byte nrfWrite(byte offset, byte data){
	byte check = 0;

	bitWriteB(CSN, 0);
	check = spiTransfer(offset + 32);
	spiTransfer(data);
	bitWriteB(CSN, 1);

	return check;
}

byte nrfRead(byte offset){
	byte check = 0;

	bitWriteB(CSN, 0);
	spiTransfer(offset);
	check = spiTransfer(0);
	bitWriteB(CSN, 1);

	return check;
}

byte nrfSend(byte data){
	byte check = 0;

	bitWriteB(CSN, 0);
	check = spiTransfer(data);
	bitWriteB(CSN, 1);

	return check;
}

byte nrfWriteBuff(byte offset, byte *data, byte size){
	byte check = 0;

	bitWriteB(CSN, 0);
	check = spiTransfer(offset + 32);
	for(byte i = 0; i < size; i++)
		spiTransfer((*data)++);
	bitWriteB(CSN, 1);

	return check;
}

byte nrfReadBuff(byte offset, byte *data, byte size){
	byte check = 0;

	bitWriteB(CSN, 0);
	check = spiTransfer(offset);
	for(byte i = 0; i < size; i++){
		*data = spiTransfer(0);
		data++;
	}
	bitWriteB(CSN, 1);

	return check;
}

////////////////////////////////////////////////////////////// LEVEL 2 INSTRUCTIONS

void nrfBitWrite(byte offset, byte bit, byte val){
	byte data;

	data = nrfRead(offset);
	if(val)
		data |= 1 << bit;
	else
		data &= ~(1 << bit);
	nrfWrite(offset, data);
}

byte nrfBitRead(byte offset, byte bit){
	byte check = 0;

	if(nrfRead(offset) & (1 << bit))
		check = 1;

	return check;
}

void SPI(byte packageSize){
	byte TX_ADDRESS[5]  = { 0x34, 0x43, 0x10, 0x10, 0x01 };

	delayms(100);
	directB(0b00101110);
	bitWriteB(IRQ, 1);
	bitWriteB(CE, 1);
	bitWriteB(CSN, 1);
	SPCR = 0b01010000;	// SPE0, MSTR0 = 1
	SPSR = 0b00000001;	// SPI2X0 = 1

	bitWriteB(CE, 0);
	nrfWrite(CONFIG,		0x7F);	// 0b01111111
	nrfWrite(EN_AA,		0);
	nrfWrite(EN_RXADDR,	1);
	nrfWrite(SETUP_AW,	3);
	nrfWrite(SETUP_RETR,	0);
	nrfWrite(RF_CH,		0x3F);	// 0b00111111
	nrfWrite(RF_SETUP,	0x2F);	// 0b00101111
	nrfWrite(STATUS,		0x7E);	// 0b01111110

	nrfWriteBuff(RX_ADDR_P0, TX_ADDRESS, 5);
	nrfWriteBuff(TX_ADDR, TX_ADDRESS, 5);
	nrfWrite(RX_PW_P0, packageSize);
	bitWriteB(CE, 1);

	nrfSend(FLUSH_TX);
	nrfSend(FLUSH_RX);
	delayus(1700);
}

////////////////////////////////////////////////////////////// LEVEL 3 INSTRUCTIONS

//////$$$$$
byte nrfTxCheck(byte clear){
	byte check = 0;

	check = nrfBitRead(0x07, 5);
	if(clear)
		nrfBitWrite(0x07, 5, 1); // <-- essa merda nao funciona

	return check;
}

//////$$$$$
byte nrfTransmit(byte *data, byte size){
	byte check = 0;

	bitWriteB(CSN, 0);
	check = spiTransfer(W_TX_PAYLOAD);	// load TX payload
	for(byte i = 0; i < size; i++)
		spiTransfer(*data++);
	bitWriteB(CSN, 1);

	/*bitWriteB(CE, 0);
	nrfBitWrite(0x00, 0, 0);	// TX mode
	bitWriteB(CE, 1);
	delayus(150);

	while(!nrfTxCheck());
	nrfBitWrite(0x00, 0, 1);	// RX mode
	bitWriteB(CE, 0);
	delayus(15);
	bitWriteB(CE, 1);*/

	return check;
}

byte nrfRxCheck(byte clear){
	byte check = 0;

	check = nrfBitRead(0x07, 6);
	if(clear)
		nrfBitWrite(0x07, 6, 1);

	return check;
}

byte nrfReceive(byte *data, byte size){
	byte check = 0;

	bitWriteB(CSN, 0);
	check = spiTransfer(R_RX_PAYLOAD);
	for(byte i = 0; i < size; i++)
		data[i] = spiTransfer(0);
	bitWriteB(CSN, 1);

	return check;
}

byte nrfRPD(){
	delayus(180);
	return nrfRead(RPD);
}
