#ifndef __NRF24
#define __NRF24

#include <Arduino.h>
#include <SPI.h>

#define CE_NRF  2
#define CS_NRF  9

#define SPI_MI  12
#define SPI_MO  11
#define SPI_CK  13

#define PACKETS 1

uint8_t nrf_channel(uint8_t);

void nrf_setup(void);

#endif