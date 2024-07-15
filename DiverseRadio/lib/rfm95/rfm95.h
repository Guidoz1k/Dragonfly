#ifndef __RFM95_H
#define __RFM95_H

#include <stdbool.h>
#include <stdint.h>

// ========== FUNCTION PROTOTYPES ==========

uint8_t rfm_read_reg(uint8_t reg);

void rfm_write_reg(uint8_t reg, uint8_t data);

bool rfm_bitread(uint8_t address, uint8_t bit);

void rfm_bitwrite(uint8_t address, uint8_t bit, bool value);

void rfm_TXwrite(uint8_t *payload);

void rfm_RXread(uint8_t *payload);

// ============ EXTERNAL SPI FUNCTIONS ============

void rfm_setup(bool test);

uint8_t returns_regs(uint8_t *pointer);

void rfm_channel(uint8_t channel);

void rfm_payload_size(uint8_t packets);

void rfm_mode_standby(void);

void rfm_mode_activeRX(void);

bool rfm_TXtransmit(uint8_t *payload);

bool rfm_RXreceive(uint8_t *payload);

#endif /* __RFM95_H */