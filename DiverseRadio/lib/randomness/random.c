#include "random.h"

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <esp_random.h>
#include <bootloader_random.h>

// ============ EXTERNAL FUNCTIONS ============

void random_setup(void){
    bootloader_random_enable();
}

uint32_t random_32(void){
    return esp_random();
}

bool random_1(void){
    bool state = false;

    if((esp_random() & 1) == 1)
        state = true;

    return state;
}

uint8_t random_n(uint32_t limit){
    return esp_random() % limit;
}
