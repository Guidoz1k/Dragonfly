// purely for visual relief
#include "main.h"

// ========== GLOBAL VARIABLES ==========

// incremented roughly every 10µs
volatile uint64_t time_counter = 0;

// ============ CORE FUNCTIONS ============

// core 1 asynchronous task
void task_core1(void){
    #ifdef ENV_BASE
        while(1){
            delay_milli(1000);
        }
    #elif ENV_DRONE
        while(1){
            delay_milli(1000);
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
        uint8_t tx_buffer = 0;
        bool test = false;

        while(1){
            serial_write_string(" ready? \n ", false);
            while(serial_read_singlechar() != 'y')
                delay_milli(10);
            serial_write_string(" LESGO ", true);
            test = true;
            tx_buffer = 0;

            while(test == true){
                if(tx_buffer < 255){
                    serial_write_byte(tx_buffer, HEX, true);
                    rfm_TXtransmit(&tx_buffer);
                    tx_buffer++;
                }
                else{
                    test = false;
                }
                delay_milli(100);
            }
        }
    #elif ENV_DRONE
        static uint8_t rx_old = 255;
        static uint8_t rx_new = 0, counter = 0, loss = 0;
        static uint32_t timeout = 0;
        bool test = false;
    
        while(1){
            rx_old = 255;
            rx_new = 0;
            counter = 0;
            loss = 0;
            timeout = 0;
            test = true;
            while(test == true){
                if(rfm_RXreceive(&rx_new) == true){
                    timeout = 0;
                    counter++;
                    if(rx_new != (rx_old + 1)){
                        loss += rx_new - (rx_old + 1);
                        serial_write_string(" LOSS: ", false);
                        serial_write_byte(loss, DEC, false);
                        serial_write_string(" / ", false);
                        serial_write_byte(counter, DEC, true);
                    }
                    rx_old = rx_new;
                    
                    //serial_write_byte(rx_new, HEX, true);
                }
                else{
                    timeout++;
                    if(timeout >= 1000ul){
                        test = false;
                    }
                }
                delay_tick();
            }
            serial_write_string(" TIMEOUT! ", true);
            serial_write_string("total errors:", false);
            serial_write_byte(loss, DEC, false);
            serial_write_string("/", false);
            serial_write_byte(0xFE, DEC, false);
            serial_write_string(" Resetting... ", true);
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
