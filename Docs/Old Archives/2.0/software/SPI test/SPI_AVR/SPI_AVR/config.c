#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"

void nopi(){
	asm volatile("nop");
}