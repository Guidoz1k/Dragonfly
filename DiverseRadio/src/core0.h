#ifndef __CORE0_H
#define __CORE0_H

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include <esp_intr_alloc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "delay.h"

#define PERIOD0     50      // interrupt period in µs
#define SYNCPIN0    32      // interruption 0 work pin

void task_core0BASE(void);

void task_core0DRONE(void);

void timer_core0_setup(bool is_base);

#endif /* __CORE0_H */