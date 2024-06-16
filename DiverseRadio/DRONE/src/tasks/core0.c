#include "core0.h"

#include "nrf24/nrf24.h"
#include "serial/serial.h"

void task_core0(void){
    uint8_t dump[11] = {0};
    uint8_t dump2[11] = {0};
    uint8_t reg_address[11] = {
        0x00,
        0x01,
        0x02,
        0x03,
        0x04,
        0x05,
        0x06,
        0x07,
        0x09,
        0x11,
        0x17
    };
    uint8_t i = 0;
    uint8_t rx_boy = 0;

    delay_milli(2000);
    while(1){
        nrf_dump11reg(dump2);
        for(i = 0; i < 11; i++){
            if(dump[i] != dump2[i]){
                serial_write_string("DIFF FOUND: reg ", false);
                serial_write_byte(reg_address[i], HEX, false);
                serial_write_string(", previous value = ", false);
                serial_write_byte(dump[i], BIN, false);
                serial_write_string(", current value = ", false);
                serial_write_byte(dump2[i], BIN, true);
                dump[i] = dump2[i];
            }
        }
        if(nrf_RXreceive(&rx_boy) == true){
            serial_write_string(" PACKAGE DETECTED!! value: ", false);
            serial_write_byte(rx_boy, HEX, true);
        }
        delay_milli(1000);
    }
}

bool IRAM_ATTR timer_core0(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 1);

    // TASK ON CORE 0

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 0);
    return true; // return true to yield for ISR callback to continue
}

void timer_core0_setup(void){
    // GPIO config variables
    gpio_config_t outputs = {
        .pin_bit_mask = (uint64_t)1 << SYNCPIN0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // timer and alarm variables
    gptimer_handle_t timer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // use default clock source
        .direction = GPTIMER_COUNT_UP,      // count up
        .resolution_hz = 1000000,           // 1 MHz, 1 tick = 1 us
    };
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_core0,        // set callback for alarm event
    };
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = PERIOD0,             // 1 millisecond
        .reload_count = 0,                  // no initial reload
        .flags.auto_reload_on_alarm = true, // automatically reload counter on alarm
    };

    // GPIO config commands
    gpio_config(&outputs);
    gpio_set_level(SYNCPIN0, 0);

    // timer and alarm commands
    gptimer_new_timer(&timer_config, &timer);
    gptimer_register_event_callbacks(timer, &cbs, NULL);
    gptimer_set_alarm_action(timer, &alarm_config);
    gptimer_enable(timer);
    gptimer_start(timer);
}
