/* ABOUT
    Radio 1 is an EBYTE E01-ML01DP5 utilizing a Nordic Semiconductor nRF24L01
    Radio 2 is a HOPERF RFM95W utilizing a Semtech SX1276
*/
// purely for visual relief
#include "main.h"

// ========== GLOBAL VARIABLES ==========

// incremented roughly every 20µs
static volatile uint64_t time_counter = 0;

/* communication packet header
    byte 1 - system command
    byte 2 - message counter
    byte 3 - channel of next message
*/
static uint8_t header[3] = {0};
static uint8_t receiving_buffer[3] = {0};

#ifdef ENV_BASE // Base system global variables

    static uint8_t packet_counter = 0;
    static bool diversity = false;
    static bool hopping = false;
    static bool radio1_status[NRF_MAX] = {0};
    static bool radio2_status[RFM_MAX] = {0};

#elif ENV_DRONE// Drone system global variables

    static volatile uint64_t timeout_start = 0;     // first occurence of no response
    static const uint64_t timeout_max = 150000ul;    // maximum timer ticks (20µs) to clear Drone status = 3s
    static uint8_t active_radio = RADIO_NRF24;

#endif

// ============ AUXILIARY FUNCTIONS ============

void test_init(void){
    nrf_channel(0);
    nrf_payload_size(3);
    rfm_channel(0);
    rfm_payload_size(3);
}

#ifdef ENV_BASE // Base system auxiliary functions

    void testies(uint8_t radio, bool success){
        if(success == true){
            serial_write_string("==> MESSAGE :", false);
            serial_write_byte(radio, DEC, true);
            serial_write_byte(0, DEC, false);
            serial_write_string(" - ", false);
            serial_write_byte(receiving_buffer[0], DEC, true);
            serial_write_byte(1, DEC, false);
            serial_write_string(" - ", false);
            serial_write_byte(receiving_buffer[1], DEC, true);
            serial_write_byte(2, DEC, false);
            serial_write_string(" - ", false);
            serial_write_byte(receiving_buffer[2], DEC, true);
            
            led_color(0, 25, 0);
        }
        else{
            led_color(25, 0, 0);
        }
        delay_milli(100);
        led_color(0, 0, 0);
    }

    bool ping_nrf(void){
        bool test = false;
        uint8_t i;

        nrf_TXtransmit(header);

        delay_tick(); // transmission timeout of 10ms

        test = nrf_RXreceive(receiving_buffer);
        /*
        if(test == true)
            for(i = 0; i < 3; i++)
                if(header[i] != receiving_buffer[i])
                    test = false;
                    */

        return test;
    }

    bool ping_rfm(void){
        bool test = false;
        uint8_t i;

        rfm_TXtransmit(header);

        delay_tick(); // transmission timeout of 10ms

        test = rfm_RXreceive(receiving_buffer);
        /*
        if(test == true)
            for(i = 0; i < 3; i++)
                if(header[i] != receiving_buffer[i])
                    test = false;
                    */

        return test;
    }

    void radio_init_test(void){
        uint8_t i;
        uint8_t startup_tries = 0;

        header[1] = 0xFF;

        header[0] = RADIO_NRF24;
        while((ping_nrf() == false) && (startup_tries != 10)){
            startup_tries++;
            delay_tick();
        }
        if(startup_tries < 10){ // initial nrf channel 0 comms established
            for(i = 0; i < NRF_MAX; i++){
                header[2] = i + 1;
                if(i == (NRF_MAX - 1)){ // last channel
                    header[0] = RADIO_RFM95;
                    header[2] = 0;
                }
                nrf_channel(i);
                radio1_status[i] = ping_nrf();
            }

            for(i = 0; i < RFM_MAX; i++){
                header[2] = i + 1;
                if(i == (NRF_MAX - 1)){ // last channel
                    header[0] = RADIO_NRF24;
                    header[2] = 0;
                }
                rfm_channel(i);
                radio2_status[i] = ping_rfm();
            }
        }
    }

#elif ENV_DRONE// Drone system auxiliary functions

#endif

// ============ CORE FUNCTIONS ============

// core 0 asynchronous task
void task_core0(void){
    test_init();

    #ifdef ENV_BASE

        uint8_t i = 0;

        while(1){
            serial_write_string(" ready?", true);
            while(serial_read_singlechar() != 'y')
                delay_milli(10);
            //serial_write_string(" link test! ", true);
            //radio_init_test();

            serial_write_string(" comms test ", true);

            for(i = 0; i < 2; i++){
                header[0] = RADIO_RFM95;
                header[1] = 0xFF;
                header[2] = 0;
                testies(RADIO_NRF24, ping_nrf());
                header[0] = RADIO_NRF24;
                header[1] = 0xFF;
                header[2] = 0;
                testies(RADIO_RFM95, ping_rfm());
            }

            serial_write_string(" test finished \n", true);

            delay_tick();
        }

    #elif ENV_DRONE

        bool radio_status = false;

        while(1){
            switch(active_radio){
            case RADIO_NRF24:
                radio_status = nrf_RXreceive(receiving_buffer);
                break;
            case RADIO_RFM95:
                radio_status = rfm_RXreceive(receiving_buffer);
                break;
            }
            if(radio_status == true){   // MESSAGE RECEIVED
                timeout_start = 0;
                header[0] = receiving_buffer[0];
                header[1] = receiving_buffer[1];
                header[2] = receiving_buffer[2];

                serial_write_string("==> MESSAGE :", false);
                serial_write_byte(active_radio, DEC, true);
                serial_write_byte(0, DEC, false);
                serial_write_string(" - ", false);
                serial_write_byte(receiving_buffer[0], DEC, true);
                serial_write_byte(1, DEC, false);
                serial_write_string(" - ", false);
                serial_write_byte(receiving_buffer[1], DEC, true);
                serial_write_byte(2, DEC, false);
                serial_write_string(" - ", false);
                serial_write_byte(receiving_buffer[2], DEC, true);

                switch(active_radio){
                case RADIO_NRF24:
                    nrf_TXtransmit(header);
                    nrf_channel(receiving_buffer[2]);
                    break;
                case RADIO_RFM95:
                    rfm_TXtransmit(header);
                    rfm_channel(receiving_buffer[2]);
                    break;
                }
                active_radio = receiving_buffer[0];
                led_color(0, 255, 0);
                delay_tick();
                led_color(0, 0, 0);
            }
            else{                       // MESSAGE NOT RECEIVED
                if(timeout_start != 0){ // IN SILENCE ALREADY
                    if( (time_counter - timeout_start) >= timeout_max ){
                        timeout_start = 0;
                        active_radio = RADIO_NRF24;
                        nrf_channel(0);
                        rfm_channel(0);
                        led_color(255, 0, 0);
                        delay_tick();
                        led_color(0, 0, 0);
                        serial_write_string("\n RESET", true);
                    }
                }
                else{   // FIRST TIME IN SILENCE
                    timeout_start = time_counter;
                }
            }
            delay_tick();
        }

    #endif
}

// core 1 timer interrupt subroutine
bool IRAM_ATTR timer_core1(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 1);

    time_counter++;

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN1, 0);
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

/* CORE 0 TIMER INTERRUPT DISABLED
// core 0 timer interrupt subroutine
bool IRAM_ATTR timer_core0(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 1);

    #ifdef ENV_BASE
        // BASE SYSTEM CODE
    #elif ENV_DRONE
        // DRONE SYSTEM CODE
    #endif

    // makes it easy to measure interruption time
    gpio_set_level(SYNCPIN0, 0);
    return true; // return true to yield for ISR callback to continue
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
*/

/*TASK 1 DISABLED. CORE 1 USED ONLY FOR INTERRUPT
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
*/

 // RUNS ON CORE 1
void core1Task(void* parameter){
    timer_core1_setup();

    //task_core1(); TASK 1 DISABLED. CORE 1 USED ONLY FOR INTERRUPT

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
    random_setup();
    //timer_core0_setup(); CORE 0 TIMER INTERRUPT DISABLED

    // core 1 task creation
    xTaskCreatePinnedToCore(core1Task, "timer1Creator", 10000, NULL, 1, NULL, 1);

    
    delay_milli(1000); // gives time to finish the logging
    task_core0();

    // core 0 task termination
    vTaskDelete(NULL);
}
