#ifndef __SPI_H
#define __SPI_H

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include <esp_intr_alloc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "hal/spi_types.h"
#include "driver/spi_master.h"

#define PIN_MOSI    23
#define PIN_MISO    19
#define PIN_CLK     18
#define PIN_CS      17
#define PIN_CE      16

#define NRF_CMD_W_REGISTER 0x20
#define NRF_CMD_R_REGISTER 0x00

#endif /* __SPI_H */