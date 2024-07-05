#ifndef __RFM95_H
#define __RFM95_H

#include <stdbool.h>
#include <stdint.h>

// ========== FUNCTION PROTOTYPES ==========

uint8_t rfm_read_reg(uint8_t reg);

void rfm_write_reg(uint8_t reg, uint8_t data);

bool rfm_bitread(uint8_t address, uint8_t bit);

void rfm_bitwrite(uint8_t address, uint8_t bit, bool value);

// ============ EXTERNAL SPI FUNCTIONS ============

void rfm_setup(bool test);

void rfm_frequency(uint32_t freq);

bool rfm_TXtransmit(uint8_t *payload);

#endif /* __RFM95_H */