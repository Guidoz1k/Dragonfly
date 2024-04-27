#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "serial.h"
#include "ports.h"
#include "spi.h"
#include "timers.h"

#define PACKAGE	32

char slashN[] = { 13, 10, 0 };
char slashT[] = { 32, 9, 32, 0 };

byte buffer[PACKAGE];

byte status = 0;

ISR(TIMER0_COMPA_vect){
	bitToggleD(2);
}

void init(){
	serialInit();
	delayInit();
	directC(0xFF);
	SPI(PACKAGE);
	//interruptInit();
	for(int i = 0; i < 32; i++)
		buffer[i] = i;
}

// Register scanner
void nrfRegScan(){
/*	for(int i = 0; i <= 7; i++){
		serialWriteString("reg: ");
		serialWriteNumber(i);
		serialWriteString(" = ");
		serialWriteBinary(nrfRead(i));
		serialWriteString(slashN);
	}*/
	for(byte i = 0; i < 0x1D; i++){
		serialWriteNumber(nrfRead(i));
		serialWriteString(" ");
	}
	serialWriteString(slashN);
}

int main(){
	byte status = 0;

	init();
	nrfTransmit(buffer, PACKAGE);

	while(1){

// TX

	for(byte i = 0; i < PACKAGE; i++)
		buffer[i] += 32;
	status = nrfRead(STATUS);
	if(status & 0x20){
		nrfWrite(FLUSH_TX, 0);
		nrfTransmit(buffer, PACKAGE);
	}
	//nrfWrite(STATUS, status);
	delayms(100);delayms(100);delayms(100);delayms(100);delayms(100);
	delayms(100);delayms(100);delayms(100);delayms(100);delayms(100);
	//nrfRegScan();

}
}
