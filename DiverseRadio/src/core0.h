#ifndef __CORE0_H
#define __CORE0_H

#include <stdio.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <esp_intr_alloc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "delay.h"
#include "nrf24.h"
#include "serial.h"
#include "led.h"

#define PERIOD0     10      // interrupt period in µs
#define SYNCPIN0    47      // interruption 0 work pin

void task_core0BASE(void);

void task_core0DRONE(void);

void timer_core0_setup(bool is_base);

#endif /* __CORE0_H */