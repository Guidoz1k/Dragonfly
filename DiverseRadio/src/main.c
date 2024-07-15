// purely for visual relief
#include "main.h"

// ========== GLOBAL VARIABLES ==========

// incremented roughly every 10µs
volatile uint64_t time_counter = 0;

// ============ CORE FUNCTIONS ============

void debug(void){
    uint8_t i = 0;
    static uint8_t old_vals[71] = {0};
    uint8_t new_vals[71] = {0};
    //static uint8_t old_gpios[4] = {0};
    //uint8_t new_gpios[4] = {0};
    //uint8_t gpios[4] = { 16, 17, 18, 8 };

    for(i = 1; i <= 0x3F; i++){
        if((i != 0x11) && (i != 0x1D)  && (i != 0x1E) && (i != 0x3C)){
            new_vals[i] = rfm_read_reg(i);
            if(new_vals[i] != old_vals[i]){
                serial_write_string(" REG: ", false);
                serial_write_byte(i, HEX, false);
                serial_write_string("; OLD = ", false);
                serial_write_byte(old_vals[i], BIN, false);
                serial_write_string("; NEW = ", false);
                serial_write_byte(new_vals[i], BIN, true);
                old_vals[i] = new_vals[i];
            }
        }
    }
    /*
    for(i = 0; i < 4; i++){
        new_gpios[i] = !gpio_get_level(gpios[i]);
        if(new_gpios[i] != old_gpios[i]){
            serial_write_string(" DIO: ", false);
            serial_write_byte(i, DEC, false);
            serial_write_string("; OLD = ", false);
            serial_write_byte(new_gpios[i], DEC, false);
            serial_write_string("; NEW = ", false);
            serial_write_byte(old_gpios[i], DEC, true);
            old_gpios[i] = new_gpios[i];
        }
    }
    */
}

// core 1 asynchronous task
void task_core1(void){
    #ifdef ENV_BASE
        while(1){
            delay_milli(1000);
        }
    #elif ENV_DRONE
        uint8_t rx_buffer = 0;

        while(1){
            if(rfm_RXreceive(&rx_buffer) == true){
                serial_write_string(" RFM95: ", false);
                serial_write_byte(rx_buffer, HEX, true);
            }
            if(nrf_RXreceive(&rx_buffer) == true){
                serial_write_string(" nRF24: ", false);
                serial_write_byte(rx_buffer, HEX, true);
            }
            delay_tick();
        }
    #endif
}

// core 1 timer interrupt subroutine
bool IRAM_ATTR timer_core1(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 1);

    #ifdef ENV_BASE
        // BASE SYSTEM CODE
    #elif ENV_DRONE
        // DRONE SYSTEM CODE
    #endif

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 0);
    return true; // return true to yield for ISR callback to continue
}

// core 0 asynchronous task
void task_core0(void){
    #ifdef ENV_BASE
        uint8_t tx_buffer = 0x69;

        serial_write_string(" ready? \n ", false);
        while(serial_read_singlechar() != 'y')
            delay_milli(10);
        serial_write_string(" LESGO ", true);

        while(1){
            rfm_TXtransmit(&tx_buffer);
            tx_buffer++;
            delay_milli(250);
        }
    #elif ENV_DRONE
        while(1){
            led_color(0, 0, 0);
            delay_milli(1000);
        }
    #endif
}

// core 0 timer interrupt subroutine
bool IRAM_ATTR timer_core0(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 1);

    time_counter++;

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 0);
    return true; // return true to yield for ISR callback to continue
}

// interrupt timer configuration for core 1 interrupt
void timer_core1_setup(void){
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
        .on_alarm = timer_core1, // setting the  callback for alarm event for BASE and DRONE systems
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
    gptimer_enable(timer);
    gptimer_start(timer);
}

// interrupt timer configuration for core 0 interrupt
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
        .on_alarm = timer_core0, // set callback for alarm event
    };
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = PERIOD0,             // 10 microseconds
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

 // RUNS ON CORE 1
void core1Task(void* parameter){
    timer_core1_setup();

    task_core1();

    // core 1 task termination
    vTaskDelete(NULL);
}

 // RUNS ON CORE 0
void app_main(){
    delay_milli(1000);
    led_setup();
    serial_setup();
    nrf_setup();
    rfm_setup();
    timer_core0_setup();

    // core 1 task creation
    xTaskCreatePinnedToCore(core1Task, "timer1Creator", 10000, NULL, 1, NULL, 1);

    task_core0();

    // core 0 task termination
    vTaskDelete(NULL);
}
