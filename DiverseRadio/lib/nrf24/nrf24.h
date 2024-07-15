#ifndef __NRF24_H
#define __NRF24_H

#include <stdbool.h>
#include <stdint.h>

void nrf_setup(void);

void nrf_channel(uint8_t channel);

void nrf_payload_size(uint8_t packets);

bool nrf_RPD_check(void);

void nrf_mode_standby(void);

void nrf_mode_activeRX(void);

bool nrf_RXreceive(uint8_t *payload);

bool nrf_TXtransmit(uint8_t *payload);

#endif /* __NRF24_H */