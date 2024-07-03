/* old library using rmt.h
#define RMT_CHANNEL RMT_CHANNEL_0
#define RMT_CLK_DIV 2  // Counter clock 40MHz / 2 = 20MHz

// WS2812 timing parameters
#define T0H 14  // 0 bit high time (400ns)
#define T0L 32  // 0 bit low time (850ns)
#define T1H 32  // 1 bit high time (850ns)
#define T1L 14  // 1 bit low time (400ns)
*/
#ifndef __LED
#define __LED

#include <stdio.h>
#include <driver/gpio.h>
#include <driver/dedic_gpio.h>

#include "delay.h"

// Define the GPIO pin connected to the LED
#define LED_GPIO_PIN 48

void led_setup(void);

void led_color(uint8_t r, uint8_t g, uint8_t b);

#endif /* __LED */