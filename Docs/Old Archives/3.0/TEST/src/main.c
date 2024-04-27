#include <avr/io.h>
#include <avr/interrupt.h>

#include "nop.h"
#include "timers.h"
#include "ports.h"
#include "serial.h"

ISR(TIMER0_COMPA_vect){
    bitWriteC(0, 1);

    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();
    nop();

    bitWriteC(0, 0);
}

void config(void){
    bitDirectB(5, 1);
    bitDirectC(0, 1);
    interruptInit(2, 20);
    delayInit();
    serialInit9600();
}

int main(void){
config();
while(1){
    bitWriteB(5, 0);
    delayms(500);
    bitWriteB(5, 1);
    delayms(500);
    serialWrite(48);
}}
