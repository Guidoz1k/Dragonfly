#include "core0.h"

void task_core0BASE(void){
    enum {
        RED = 0,
        GREEN = 1,
        BLUE = 2,
    } mode;
    uint8_t r_output;
    uint8_t g_output;
    uint8_t b_output;

    mode = GREEN;
    r_output = 255;
    g_output = 0;
    b_output = 0;

    while(1){
        switch(mode){
        case RED:
            led_color(r_output++, 0, b_output--);
            if(r_output == 255)
                mode = GREEN;
            break;
        case GREEN:
            led_color(r_output--, g_output++, 0);
            if(g_output == 255)
                mode = BLUE;
            break;
        case BLUE:
            led_color(0, g_output--, b_output++);
            if(b_output == 255)
                mode = RED;
            break;
        }
        delay_milli(10);
    }
}

void task_core0DRONE(void){
    uint8_t tx_buffer = 0;
    uint8_t rx_buffer = 0;

    delay_milli(1000);
    serial_write_string(" READY TO RECEIVE ", true);

    while(1){    
        while(nrf_RXreceive(&rx_buffer) == false)
            delay_milli(10);
        if(rx_buffer == 0x77){
            serial_write_string(" MESSAGE RECEIVED: ", false);
            serial_write_byte(rx_buffer, HEX, true);
            tx_buffer = 0xF0;
        }
        nrf_TXtransmit(&tx_buffer);
    }
}

bool IRAM_ATTR timer_core0BASE(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 1);

    // TASK ON CORE 0

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 0);
    return true; // return true to yield for ISR callback to continue
}

bool IRAM_ATTR timer_core0DRONE(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 1);

    // TASK ON CORE 0

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 0);
    return true; // return true to yield for ISR callback to continue
}

void timer_core0_setup(bool is_base){
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
        .on_alarm = (is_base == true) ? timer_core0BASE : timer_core0DRONE, // set callback for alarm event
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
