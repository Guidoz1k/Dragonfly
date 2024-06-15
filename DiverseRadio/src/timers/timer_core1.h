#ifndef __TIMER_CORE1_H
#define __TIMER_CORE1_H

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include <esp_intr_alloc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PERIOD1     100     // interrupt period in µs
#define SYNCPIN1    33      // interruption 1 work pin

void timer_core1_setup(void);

#endif /* __TIMER_CORE1_H */