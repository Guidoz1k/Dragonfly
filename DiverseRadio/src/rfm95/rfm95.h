#ifndef __RFM95
#define __RFM95

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include <esp_intr_alloc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#define CS_RFM  6

#define SPI_MI  12
#define SPI_MO  11
#define SPI_CK  13
/*
uint8_t spi_rfm(void);

bool rfm_setup(void);
*/
#endif