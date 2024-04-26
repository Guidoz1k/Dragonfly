#ifndef __DIVERSESPI
#define __DIVERSESPI

#include <Arduino.h>
#include <SPI.h>

#define CS_NRF  9
#define CE_NRF  2
#define CS_RFM  6
#define SPI_MI  12
#define SPI_MO  11
#define SPI_CK  13

uint8_t spi_nrf();

uint8_t spi_rfm();

void diverse_setup(void);

#endif
