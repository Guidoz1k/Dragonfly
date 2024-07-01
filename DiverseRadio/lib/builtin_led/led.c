#include "led.h"

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_t;

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
