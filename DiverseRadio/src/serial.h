#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include <esp_intr_alloc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "driver/uart.h"

#define MAXSIZE 32

void serial_setup();

void serial_write_number(int_fast32_t number, uint_fast8_t newline);

void serial_write_hex(uint_fast32_t number);

void serial_new_line(void);

void serial_write_string(const char *pointer, uint8_t newline);

#endif /* SERIAL */