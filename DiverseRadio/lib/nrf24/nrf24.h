#ifndef __NRF24_H
#define __NRF24_H

#include <stdio.h>
#include "driver/gpio.h"
#include "hal/spi_types.h"
#include "driver/spi_master.h"

#include "delay.h"

// ========== DEFINITIONS ==========

#define PIN_MOSI    9
#define PIN_MISO    10
#define PIN_CLK     11
#define PIN_CS      12
#define PIN_CE      13

#define PAYLOADSIZE 1
#define STANDARDCH  0x3F

// ========== FUNCTION PROTOTYPES ==========

void nrf_setup(bool test);

void nrf_dump11reg(uint8_t *reg_p);

void nrf_channel(uint8_t channel);

void nrf_payload_size(uint8_t packets);

bool nrf_RPD_check(void);

void nrf_mode_standby(void);

void nrf_mode_activeRX(void);

bool nrf_RXreceive(uint8_t *payload);

bool nrf_TXtransmit(uint8_t *payload);

#endif /* __NRF24_H */