#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include <driver/uart.h>

#define MAXSIZE 32
#define MAXWAIT 50  // 10 milliseconds to read buffer

typedef enum {
    BIN = 2,
    DEC = 10,
    HEX = 16,
} base_t;

void serial_setup();

uint8_t serial_write_word(uint32_t number, uint8_t size, bool newline);

void serial_write_byte(uint8_t number, base_t base, bool newline);

void serial_new_line(void);

void serial_write_string(const char *pointer, bool newline);

uint8_t serial_read_chars(uint8_t *buffer, uint8_t size);

char serial_read_singlechar(void);

uint8_t serial_read_size(void);

void serial_flush(void);

#endif /* __SERIAL_H */