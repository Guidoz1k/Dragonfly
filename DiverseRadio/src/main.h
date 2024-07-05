#ifndef __MAIN_H
#define __MAIN_H

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <esp_intr_alloc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>

// ========== INTERNAL LIBRARIES ==========

#include "delay.h"
#include "nrf24.h"
#include "rfm95.h"
#include "serial.h"
#include "led.h"

// ========== DEFINITIONS ==========

// environment test for power transmission setup
#ifdef ENV_BENCHTEST
    #define BENCHTESTING true
#else
    #define BENCHTESTING false
#endif

#define PERIOD0     10      // interrupt period in µs
#define SYNCPIN0    47      // interruption 0 work pin
#define PERIOD1     100     // interrupt period in µs
#define SYNCPIN1    21      // interruption 1 work pin

#endif /* __MAIN_H */