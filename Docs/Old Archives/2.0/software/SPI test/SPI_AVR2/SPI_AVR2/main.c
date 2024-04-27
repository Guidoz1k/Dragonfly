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
	for(int i = 0; i <= 7; i++){
		serialWriteString("reg: ");
		serialWriteNumber(i);
		serialWriteString(" = ");
		serialWriteBinary(nrfRead(i));
		serialWriteString(slashN);
	}
}

int main(){
	init();
	while(1){

// RX
	if(nrfRxCheck(1)){
		nrfReceive(buffer, PACKAGE);
		nrfSend(FLUSH_RX);
		for(byte i = 0; i < PACKAGE; i++){
			serialWriteNumber(buffer[i]);
			serialWriteString(" ");
		}
		serialWriteString(slashN);
	}
	delayms(100);

}
}
