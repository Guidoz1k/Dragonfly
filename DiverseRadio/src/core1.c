#include "core1.h"

#include "nrf24.h"
#include "serial.h"

#include "esp_log.h"

static const char *TAG = "UART_EXAMPLE";

void task_core1BASE(void){
    uint8_t data[10] = {0};
    int len = 0 ;

    while(1){
        delay_milli(500);
        len = uart_read_bytes(UART_NUM_0, data, 10, 10);
        if(len > 0){
            data[len] = '\0';
            ESP_LOGI(TAG, "\n RECEIVED: %s", data);
        }
    }

    /*
    uint8_t tx_boy = 0xEA;
    uint8_t rx_buffer[32] = {0};
    bool test = false;

    delay_milli(1500);
    serial_write_string("Initiate test?", true);
    while(test == false){
        while(serial_read_size() == 0)
            delay_milli(100);
        serial_read_chars(rx_buffer, 1);
        serial_write_byte(rx_buffer[0], DEC, true);
        if(rx_buffer[0] == 'y')
            test = true;
    }

    serial_new_line();
    while(1){
        serial_write_string("TX ENGAGED! value: ", false);
        nrf_TXtransmit(&tx_boy);
        serial_write_byte(tx_boy++, HEX, true);
        delay_milli(1000);
    }
    //*/
}

void task_core1DRONE(void){
    while(1){
        delay_milli(1000);
    }
}

bool IRAM_ATTR timer_core1BASE(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 1);

    // TASK OF CORE 1

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 0);
    return true; // return true to yield for ISR callback to continue
}

bool IRAM_ATTR timer_core1DRONE(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 1);

    // TASK OF CORE 1

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 0);
    return true; // return true to yield for ISR callback to continue
}

void timer_core1_setup(bool is_base){
    // GPIO config variables
    gpio_config_t outputs = {
        .pin_bit_mask = (uint64_t)1 << SYNCPIN1,
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
        .on_alarm = (is_base == true) ? timer_core1BASE : timer_core1DRONE, // setting the  callback for alarm event for BASE and DRONE systems
    };
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = PERIOD1,             // 2 millisecond
        .reload_count = 0,                  // no initial reload
        .flags.auto_reload_on_alarm = true, // automatically reload counter on alarm
    };

    // GPIO config commands
    gpio_config(&outputs);
    gpio_set_level(SYNCPIN1, 0);

    // timer and alarm commands
    gptimer_new_timer(&timer_config, &timer);
    gptimer_register_event_callbacks(timer, &cbs, NULL);
    gptimer_set_alarm_action(timer, &alarm_config);
    //gptimer_enable(timer);
    //gptimer_start(timer);
}
