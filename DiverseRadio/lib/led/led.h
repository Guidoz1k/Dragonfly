#ifndef __LED
#define __LED

#include "esp_err.h"
#include "driver/rmt.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"

// Define the GPIO pin connected to the LED
#define LED_GPIO_PIN 48
#define RMT_CHANNEL RMT_CHANNEL_0
#define RMT_CLK_DIV 2  // Counter clock 40MHz / 2 = 20MHz

// WS2812 timing parameters
#define T0H 14  // 0 bit high time (400ns)
#define T0L 32  // 0 bit low time (850ns)
#define T1H 32  // 1 bit high time (850ns)
#define T1L 14  // 1 bit low time (400ns)

void led_init(void);

void led_color(uint8_t r, uint8_t g, uint8_t b);

#endif /* __LED */