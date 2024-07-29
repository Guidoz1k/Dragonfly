/* old library using rmt.h
// Function to convert RGB values to RMT items
void rgb_to_rmt_items(const rgb_t *rgb, rmt_item32_t *items) {
    uint32_t bits[3] = {rgb->green, rgb->red, rgb->blue}; // WS2812 requires GRB order
    for (int color = 0; color < 3; ++color) {
        uint32_t mask = 1 << 7;
        for (int bit = 0; bit < 8; ++bit) {
            if (bits[color] & mask) {
                items[color * 8 + bit].duration0 = T1H;
                items[color * 8 + bit].level0 = 1;
                items[color * 8 + bit].duration1 = T1L;
                items[color * 8 + bit].level1 = 0;
            } else {
                items[color * 8 + bit].duration0 = T0H;
                items[color * 8 + bit].level0 = 1;
                items[color * 8 + bit].duration1 = T0L;
                items[color * 8 + bit].level1 = 0;
            }
            mask >>= 1;
        }
    }
}

void led_color(uint8_t r, uint8_t g, uint8_t b){
    rgb_t rgb = {r, g, b};
    rmt_item32_t items[24];
    rgb_to_rmt_items(&rgb, items);
    rmt_write_items(RMT_CHANNEL, items, 24, true);
    rmt_wait_tx_done(RMT_CHANNEL, portMAX_DELAY);
}

void led_setup(void){
    // RMT INIT
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_GPIO_PIN, RMT_CHANNEL);
    config.clk_div = RMT_CLK_DIV;  // set counter clock to 40MHz / 2 = 20MHz
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);
    led_color(0, 0, 0);
}
// help from https://github.com/espressif/ESP8266_RTOS_SDK/issues/914
*/
#include "led.h"

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <driver/gpio.h>
#include <driver/dedic_gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>

// ========== INTERNAL LIBRARIES ==========

#include "delay.h"

// ========== DEFINITIONS ==========

// Define the GPIO pin connected to the LED
#define LED_GPIO_PIN 48

// ========== GLOBAL VARIABLES ==========

static const char *TAG = "WS2812B"; // esp_err variable

static dedic_gpio_bundle_handle_t gpio_bundle = NULL; // hardware handle

// ============ INTERNAL FUNCTIONS ============

static inline void led_gpio_fast_set(void){
    dedic_gpio_bundle_write(gpio_bundle, 1, 1);
}

static inline void led_gpio_fast_reset(void){
    dedic_gpio_bundle_write(gpio_bundle, 1, 0);
}

static inline void IRAM_ATTR ws2812b_bit_0(){
    led_gpio_fast_set();
    //delay ~ 0.3875µs
    asm("nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop;");
    led_gpio_fast_reset();
    //delay ~ 0.8375µs
    asm("nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop;");
}

static inline void IRAM_ATTR ws2812b_bit_1(){
    led_gpio_fast_set();
    //delay ~ 0.6625µs
    asm("nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop;");
    led_gpio_fast_reset();
    //delay ~ 0.5625us
    asm("nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;"
        "nop; nop; nop; nop; nop; nop; nop; nop;");
}

static inline void IRAM_ATTR ws2812b_byte(uint8_t byte){
    register volatile uint8_t i;

    for(i = 0; i < 8; i++) {
        if(byte & 0x80)
            ws2812b_bit_1();
        else
            ws2812b_bit_0();
        byte <<= 1;
    }
    delay_micro(50);
}

// ============ EXTERNAL FUNCTIONS ============

// Mandatory setup initialization function
void led_setup(void){
    dedic_gpio_bundle_config_t bundle_config = {
        .gpio_array = (int[]) {48},
        .array_size = 1,
        .flags = {
            .in_en = 0,
            .out_en = 1,
        },
    };

    ESP_ERROR_CHECK(dedic_gpio_new_bundle(&bundle_config, &gpio_bundle));
    led_color(0, 0, 0);
    ESP_LOGI(TAG, "LED initialized");
}

// CRITICAL SECTION SUBROUTINE, MUST RUN ON CORE 0
void IRAM_ATTR led_color(uint8_t r, uint8_t g, uint8_t b){
    taskDISABLE_INTERRUPTS(); // Critical Section to preserve timing
    ws2812b_byte(g);
    ws2812b_byte(r);
    ws2812b_byte(b);
    taskENABLE_INTERRUPTS();
}
