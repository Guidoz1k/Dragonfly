#ifndef __DELAY
#define __DELAY

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

void delay_milli(uint16_t period);

void delay_micro(uint32_t microseconds);

#endif /* __DELAY */