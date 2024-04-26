#include <Arduino.h>
#include <SPI.h>

#include <diverseSPI.h>

#define DEVICE  0

#define TEST    0

void init(void){
    diverse_setup();
}

void main(){
    init();
}
