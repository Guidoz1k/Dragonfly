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
#include "random.h"

// ========== DEFINITIONS ==========

typedef enum _radio_t{  // radio identifier
    RADIO_NRF24 = 1,
    RADIO_RFM95 = 2,
} radio_t;

//#define PERIOD0     10      // interrupt period in µs
//#define SYNCPIN0    47      // interruption 0 work pin
#define PERIOD1     20      // interrupt period in µs
#define SYNCPIN1    21      // interruption 1 work pin

#define NRF_MAX     125     // top channel of the nRF24 radio
#define RFM_MAX     64      // top channel of the RFM95 radio

#endif /* __MAIN_H */