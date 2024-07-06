// purely for visual relief
#include "main.h"

// ========== GLOBAL VARIABLES ==========

// incremented roughly every 10µs
volatile uint64_t time_counter = 0;

// ============ CORE FUNCTIONS ============

// core 1 asynchronous task
void task_core1(void){
    #ifdef ENV_BASE
        uint8_t tx_buffer = 0x77;
        uint8_t rx_buffer = 0;
        uint8_t timeout_counter = 0;
        bool received = false;

        delay_milli(2000);

        while(1){
            timeout_counter = 0;
            received = false;
        
            nrf_TXtransmit(&tx_buffer);
            while( (received == false) && (timeout_counter < 10) ){
                received = nrf_RXreceive(&rx_buffer);
                delay_milli(100);
                timeout_counter++;
            }
            if(received == false)
                serial_write_string(" TIME OUT! ", true);
            else{
                serial_write_string(" MESSAGE RECEIVED: ", false);
                serial_write_byte(rx_buffer, HEX, true);
            }
        }
    #elif ENV_DRONE
        while(1){
            delay_milli(100);
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
        enum {
            RED = 0,
            GREEN = 1,
            BLUE = 2,
        } mode = GREEN;
        uint8_t r_output = 255;
        uint8_t g_output = 0;
        uint8_t b_output = 0;
        uint8_t dimmer = 30;
    
        uint8_t i;

        delay_milli(2000);
        for(i = 0x0C; i <= 0x23; i++){
            serial_write_string("REG: ", false);
            serial_write_byte(i, HEX, false);
            serial_write_string(" = ", false);
            serial_write_byte(rfm_read_reg(i), HEX, true);
        }

        while(1){
            switch(mode){
            case RED:
                led_color(r_output++ / dimmer, 0 / dimmer, b_output-- / dimmer);
                if(r_output == 255)
                    mode = GREEN;
                break;
            case GREEN:
                led_color(r_output-- / dimmer, g_output++ / dimmer, 0 / dimmer);
                if(g_output == 255)
                    mode = BLUE;
                break;
            case BLUE:
                led_color(0 / dimmer, g_output-- / dimmer, b_output++ / dimmer);
                if(b_output == 255)
                    mode = RED;
                break;
            }
            delay_milli(10);
        }
    #elif ENV_DRONE
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
    nrf_setup(BENCHTESTING);
    rfm_setup(BENCHTESTING);
    timer_core0_setup(); // interrupts disabled on core 0, for "bitbanging the led" reasons

    // core 1 task creation
    xTaskCreatePinnedToCore(core1Task, "timer1Creator", 10000, NULL, 1, NULL, 1);

    task_core0();

    // core 0 task termination
    vTaskDelete(NULL);
}
