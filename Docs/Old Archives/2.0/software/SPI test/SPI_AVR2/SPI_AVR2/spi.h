#ifndef SPI_H_
#define SPI_H_

#define MOSI	3
#define MISO	4
#define SCK		5
#define CSN		2
#define CE		1
#define IRQ		0
/*
nRF24L01+	|	ATMega328p	|	Pin		|	Arduino
MOSI			|	MOSI			|	PORTB3	|	11
MISO			|	MISO			|	PORTB4	|	12
SCK			|	SCK			|	PORTB5	|	13
CSN			|	SS				|	PORTB2	|	10
CE				|	none			|	PORTB1	|	9
IRQ			|	none			|	PORTB0	|	8
*/

// SPI commands and registers
#define FLUSH_TX		0xE1
#define FLUSH_RX		0xE2
#define CLEAR_FLAGS	0xF0
#define R_RX_PAYLOAD	0x61
#define W_TX_PAYLOAD	0xA0

#define CONFIG			0x00
#define EN_AA			0x01
#define EN_RXADDR		0x02
#define SETUP_AW		0x03
#define SETUP_RETR	0x04
#define RF_CH			0x05
#define RF_SETUP		0x06
#define STATUS			0x07
#define OBSERVE_TX	0x08
#define RPD				0x09
#define RX_ADDR_P0	0x0A
#define RX_ADDR_P1	0x0B
#define RX_ADDR_P2	0x0C
#define RX_ADDR_P3	0x0D
#define RX_ADDR_P4	0x0E
#define RX_ADDR_P5	0x0F
#define TX_ADDR		0x10
#define RX_PW_P0		0x11
#define RX_PW_P1		0x12
#define RX_PW_P2		0x13
#define RX_PW_P3		0x14
#define RX_PW_P4		0x15
#define RX_PW_P5		0x16
#define FIFO_STATUS	0x17
#define DYNPD			0x1C
#define FEATURE		0x1D

////////////////////////////////////////////////////////////// LEVEL 0 INSTRUCTIONS

byte spiTransfer(byte data);

////////////////////////////////////////////////////////////// LEVEL 1 INSTRUCTIONS

byte nrfWrite(byte offset, byte data);

byte nrfRead(byte offset);

byte nrfSend(byte data);

byte nrfWriteBuff(byte offset, byte *data, byte size);

byte nrfReadBuff(byte offset, byte *data, byte size);

////////////////////////////////////////////////////////////// LEVEL 2 INSTRUCTIONS

void nrfBitWrite(byte offset, byte bit, byte val);

byte nrfBitRead(byte offset, byte bit);

void SPI(byte packageSize);

////////////////////////////////////////////////////////////// LEVEL 3 INSTRUCTIONS

byte nrfTxCheck(byte clear);

byte nrfTransmit(byte *data, byte size);

byte nrfRxCheck(byte clear);

byte nrfReceive(byte *data, byte maxSize);

byte nrfRPD();

#endif /* SPI_H_ */