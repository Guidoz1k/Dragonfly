#ifndef __TIMER_CORE0_H
#define __TIMER_CORE0_H

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include <esp_intr_alloc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "serial.h"

#define PERIOD0     25      // interrupt period in µs
#define SYNCPIN0    32      // interruption 0 work pin

void timer_core0_setup(void);

#endif /* __TIMER_CORE0_H */