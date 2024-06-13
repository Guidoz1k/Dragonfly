#ifndef __NRF24
#define __NRF24

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include <esp_intr_alloc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#define CE_NRF  7
#define CS_NRF  8

#define SPI_MO  11
#define SPI_MI  12
#define SPI_CK  13
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
#endif