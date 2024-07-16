#ifndef __RFM95_H
#define __RFM95_H

#include <stdbool.h>
#include <stdint.h>

// ============ EXTERNAL SPI FUNCTIONS ============

void rfm_setup(void);

uint8_t returns_regs(uint8_t *pointer);

void rfm_channel(uint8_t channel);

void rfm_payload_size(uint8_t packets);

void rfm_mode_standby(void);

void rfm_mode_activeRX(void);

bool rfm_TXtransmit(uint8_t *payload);

bool rfm_RXreceive(uint8_t *payload);

#endif /* __RFM95_H */