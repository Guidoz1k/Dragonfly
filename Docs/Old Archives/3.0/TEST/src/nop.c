#include <avr/io.h>
#include <avr/interrupt.h>

void nop(void){
	asm volatile("nop");
}