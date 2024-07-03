#ifndef __RFM95_H
#define __RFM95_H

#include <stdio.h>
#include <driver/gpio.h>
#include <hal/spi_types.h>
#include <driver/spi_master.h>

#include "delay.h"

// ========== DEFINITIONS ==========

#define SPI_CH      SPI3_HOST
#define PIN_MOSI    9
#define PIN_MISO    10
#define PIN_CLK     11
#define PIN_CS      12
#define PIN_DIO0    0
#define PIN_DIO1    0
#define PIN_DIO2    0
#define PIN_DIO3    0
#define PIN_DIO4    0
#define PIN_DIO5    0

// ========== FUNCTION PROTOTYPES ==========

void rfm_setup(void);

#endif /* __RFM95_H */