#ifndef __NRF24_H
#define __NRF24_H

#include <stdio.h>
#include "driver/gpio.h"
#include "hal/spi_types.h"
#include "driver/spi_master.h"

#include "delay/delay.h"

#define PIN_MOSI    23
#define PIN_MISO    19
#define PIN_CLK     18
#define PIN_CS      17
#define PIN_CE      16

void nrf_setup(void);

uint8_t nrf_read_reg(uint8_t reg);

void nrf_write_reg(uint8_t reg, uint8_t data);

void nrf_command(uint8_t command);

/*
uint8_t nrf_channel(uint8_t);

uint8_t nrf_packets(uint8_t packets);

uint8_t nrf_flushTX(void);

uint8_t nrf_flushRX(void);

bool nrf_check_RX_buffer(void);

bool nrf_check_RPD(void);

void nrf_RX_mode(void);

void nrf_RX_read(uint8_t *buffer);

void nrf_standby_mode(void);

void nrf_TX_transmit(uint8_t *buffer);

void nrf_TX_chirp(uint data);

bool nrf_setup(void);

void nrf_registers(uint8_t *buffer);

void nrf_addressP0(uint8_t *buffer);
*/

#endif /* __NRF24_H */