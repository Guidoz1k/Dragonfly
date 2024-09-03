/* ABOUT
    Radio 1 is an EBYTE E01-ML01DP5 utilizing a Nordic Semiconductor nRF24L01
    Radio 2 is a HOPERF RFM95W utilizing a Semtech SX1276

    notes for this test:
    number of channels is limited to MAGICNUMBER of 30 for the nRF24 has problems when switching channels too far apart
    radio channels are excluded after only one fail, this could be configured
    CH0 is the standard one, this could be way different
*/
// purely for visual relief
#include "main.h"

#define MAGICMAXIMUM 30 // nrf cannot hop between channels that are too far apart

// ========== GLOBAL VARIABLES AND FUNCTIONS ==========

// incremented roughly every 20µs
static volatile uint64_t time_counter = 0;

/* communication packet header
    byte 1 - system command (RADIO OF NEXT MESSAGE [TEST])
    byte 2 - message counter
    byte 3 - channel of next message
*/
static uint8_t header[3] = {RADIO_NRF24, 0xFF, 0};
static uint8_t receiving_buffer[3] = {0};
static uint8_t active_radio = RADIO_NRF24;
static uint8_t active_channel = 0;

// BASE SYSTEM-only global variables and functions
#ifdef ENV_BASE
    static bool radio1_status[NRF_MAX + 1] = {0};
    static bool radio2_status[RFM_MAX + 1] = {0};
    static uint8_t radio1_available_channels = MAGICMAXIMUM - 1;//NRF_MAX; // doesn't include CH0
    static uint8_t radio2_available_channels = MAGICMAXIMUM - 1;//RFM_MAX; // doesn't include CH0
    static bool radio_has_channels = true;

    void var_init(void){
        uint8_t i = 0;
    
        for(i = 0; i <= NRF_MAX; i++){
            radio1_status[i] = true;
        }
        for(i = 0; i <= RFM_MAX; i++){
            radio2_status[i] = true;
        }
        radio1_available_channels = NRF_MAX;
        radio2_available_channels = RFM_MAX;
        radio_has_channels = true;
    }

    bool ping_nrf(void){
        bool test = false;

        nrf_TXtransmit(header);

        delay_tick(); // transmission timeout of 10ms
        delay_tick(); // transmission timeout of 10ms

        test = nrf_RXreceive(receiving_buffer);

        return test;
    }

    bool ping_rfm(void){
        bool test = false;

        rfm_TXtransmit(header);

        delay_tick(); // transmission timeout of 10ms
        delay_tick(); // transmission timeout of 10ms

        test = rfm_RXreceive(receiving_buffer);

        return test;
    }

    void complete_test(void){ // not used
        const uint8_t max_tries = 3;
        const uint16_t delay_success = 100;
        const uint16_t delay_failure = 1200;
        uint8_t i = 0;
        uint8_t tries = 0;

        serial_write_string("'+' to initiate self-test.", true);
        while(serial_read_singlechar() != '+'){
            delay_tick();
            led_color(0, 0, 0);
        }

        // RADIO 1 TEST
        header[0] = RADIO_NRF24;
        header[1] = 0xFF;
        for(i = 0; i <= NRF_MAX; i++){
            nrf_channel(0);
            header[2] = i;

            tries = 0;
            while( (ping_nrf() == false) && (tries < max_tries) ){
                delay_tick();
                tries++;
            }
            /*
            if(tries == max_tries)
                serial_write_string("CANT CONNECT NRFCH0", true);
            */

            nrf_channel(i);
            header[2] = 0;

            tries = 0;
            radio1_status[i] = false;
            while( (ping_nrf() == false) && (tries < max_tries) ){
                delay_tick();
                tries++;
            }

            if(tries != max_tries)
                radio1_status[i] = true;
            else
                delay_milli(delay_failure);
        
            delay_milli(delay_success);
        }
        // END OF RADIO 1 TEST

        // RADIO 2 TEST
        header[1] = 0xFF;
        nrf_channel(0);
        for(i = 0; i <= RFM_MAX; i++){
            header[0] = RADIO_RFM95;
            header[2] = i;

            tries = 0;
            while( (ping_nrf() == false) && (tries < max_tries) ){
                delay_tick();
                tries++;
            }
            /*
            if(tries == max_tries)
                serial_write_string("CANT CONNECT NRFCH0", true);
            */

            header[0] = RADIO_NRF24;
            header[2] = 0;
            rfm_channel(i);

            tries = 0;
            radio1_status[i] = false;
            while( (ping_rfm() == false) && (tries < max_tries) ){
                delay_tick();
                tries++;
            }

            if(tries != max_tries)
                radio1_status[i] = true;
            else
                delay_milli(delay_failure);
        
            delay_milli(delay_success);
        }
        // END OF RADIO 2 TEST

        serial_write_string("\n\n nRF24 initial test:", true);
        for(i = 0; i <= NRF_MAX; i++){
            serial_write_string(" channel: ", false);
            serial_write_byte(i, DEC, false);
            serial_write_string(", status:", false);
            if(radio1_status[i] == true)
                serial_write_string(" working.", true);
            else
                serial_write_string(" INOPERATIVE.", true);
        }
        serial_write_string("\n\n RFM95 initial test:", true);
        for(i = 0; i <= RFM_MAX; i++){
            serial_write_string(" channel: ", false);
            serial_write_byte(i, DEC, false);
            serial_write_string(", status:", false);
            if(radio2_status[i] == true)
                serial_write_string(" working.", true);
            else
                serial_write_string(" INOPERATIVE.", true);
        }
    }
#endif

// ============ CORE FUNCTIONS ============

// core 0 asynchronous task
void task_core0(void){
    nrf_payload_size(3);
    rfm_payload_size(3);

    #ifdef ENV_BASE

        static uint8_t channel_aux = 0;
        static uint8_t future_radio = 0;
        static uint8_t future_channel_1 = 0;
        static uint8_t future_channel_2 = 0;
        static uint8_t packet_counter = 0;
        static uint8_t packet_failures = 0;
        static bool diversity = false;
        static bool hopping = false;
        static char option = 0;
        static bool transmission_success = true;

        delay_milli(1000);

        while(1){
            var_init();
            nrf_channel(0);
            delay_tick();
            rfm_channel(0);
            delay_tick();

            serial_write_string("-> (1) Radio 1, CH0", true);
            serial_write_string("-> (2) Radio 2, CH0", true);
            serial_write_string("-> (3) Radio 1, Hopping", true);
            serial_write_string("-> (4) Radio 2, Hopping", true);
            serial_write_string("-> (5) Diversity, CH0", true);
            serial_write_string("-> (6) Diversity, Hopping\n ==>", false);
            option = 0;
            while( (option != '1') && (option != '2') && (option != '3') && (option != '4') && (option != '5') && (option != '6')){
                option = serial_read_singlechar();
                delay_milli(10);
                led_color(0, 0, 0);
                }
            switch(option){
            case '1':
                diversity = false;
                hopping = false;
                future_radio = RADIO_NRF24;
                break;
            case '2':
                diversity = false;
                hopping = false;
                future_radio = RADIO_RFM95;
                break;
            case '3':
                diversity = false;
                hopping = true;
                future_radio = RADIO_NRF24;
                break;
            case '4':
                diversity = false;
                hopping = true;
                future_radio = RADIO_RFM95;
                break;
            case '5':
                diversity = true;
                hopping = false;
                future_radio = RADIO_NRF24;
                break;
            case '6':
                diversity = true;
                hopping = true;
                future_radio = RADIO_NRF24;
                break;
            }

            active_radio = RADIO_NRF24;
            active_channel = 0;
            future_channel_1 = active_channel;
            serial_write_string("\n TESTING! Press '0' to stop.", true);

            while( (serial_read_singlechar() != '0') && (radio_has_channels == true)){
                header[0] = future_radio;
                header[1] = packet_counter;
                switch(future_radio){
                case RADIO_NRF24:
                    header[2] = future_channel_1;
                    break;
                case RADIO_RFM95:
                    header[2] = future_channel_2;
                    break;
                }

                /* DEBUG TEXTS
                serial_write_string("\n radio: ", false);
                serial_write_byte(active_radio, DEC, true);
                serial_write_string(" channel: ", false);
                serial_write_byte(active_channel, DEC, true);
                serial_write_string(" 0 - ", false);
                serial_write_byte(header[0], DEC, true);
                serial_write_string(" 1 - ", false);
                serial_write_byte(header[1], DEC, true);
                serial_write_string(" 2 - ", false);
                serial_write_byte(header[2], DEC, true);
                */

                // RUNS TRANSMISSION
                switch(active_radio){
                case RADIO_NRF24:
                    transmission_success = ping_nrf();
                    break;
                case RADIO_RFM95:
                    transmission_success = ping_rfm();
                    break;
                }
                // END OF TRANSMISSION

                /* RFM for some reason reads wrong packets sometimes, makings some packets = 0
                if( (header[0] != receiving_buffer[0]) || (header[1] != receiving_buffer[1]) || (header[2] != receiving_buffer[2]))
                    transmission_success = false;
                */

                if(transmission_success == true){   // channel is working
                    serial_write_string("S", false);
                    if(diversity == true)
                        serial_write_word(active_radio, 1, false);
                    active_radio = future_radio;
                    switch(active_radio){
                    case RADIO_NRF24:
                        active_channel = future_channel_1;
                        nrf_channel(active_channel);
                        break;
                    case RADIO_RFM95:
                        active_channel = future_channel_2;
                        rfm_channel(active_channel);
                        break;
                    }

                    // RADIO DIVERSITY CONFIG
                    if(diversity == true){
                        if(active_radio == RADIO_NRF24)
                            future_radio = RADIO_RFM95;
                        else
                            future_radio = RADIO_NRF24;
                    }
                    // END OF RADIO DIVERSITY CONFIG

                    // CHANNEL HOPPING CONFIG
                    if(hopping == true){
                        if(future_radio == RADIO_NRF24){
                            do{
                                switch(channel_aux){
                                case 0:
                                        if(future_channel_1 == MAGICMAXIMUM)
                                            channel_aux = 1;
                                        else
                                            future_channel_1++;
                                        break;
                                case 1:
                                    if(future_channel_1 == 0)
                                        channel_aux = 0;
                                    else
                                        future_channel_1--;
                                    break;
                                }
                            }
                            while(radio1_status[future_channel_1] == false);
                        }
                        else
                            do{
                                if(future_channel_2 == MAGICMAXIMUM)
                                    future_channel_2 = 0;
                                else
                                    future_channel_2++;
                            }
                            while(radio2_status[future_channel_2] == false);
                    }
                    // END OF CHANNEL HOPPING CONFIG
                }
                else{                               // channel is not working
                    serial_write_string("F", false);
                    packet_failures++;
                    if(diversity == true){
                        serial_write_word(active_radio, 1, false);
                        serial_write_string("-", false);
                    }
                    if((active_channel != 0) && (hopping == true)){ // never flags CH0 as inoperative
                        serial_write_word(active_channel, 2, false);
                        switch(active_radio){
                        case RADIO_NRF24:
                            radio1_status[active_channel] = false;
                            if(radio1_available_channels > 1)
                                radio1_available_channels--;
                            else
                                radio_has_channels = false;
                            break;
                        case RADIO_RFM95:
                            radio2_status[active_channel] = false;
                            if(radio2_available_channels > 1)
                                radio2_available_channels--;
                            else
                                radio_has_channels = false;
                            break;
                        }
                    }
                    if( (diversity == true) || (hopping == true) || (active_radio == RADIO_RFM95)){
                        delay_milli(1000);
                        if( (option == '2') || (option == '4') )
                            future_radio = RADIO_RFM95;
                        else
                            future_radio = RADIO_NRF24;
                        active_radio = RADIO_NRF24;
                        active_channel = 0;
                        future_channel_1 = 0;
                        future_channel_2 = 0;
                        nrf_channel(0);
                        rfm_channel(0);
                    }
                }

                if(packet_counter < 255)
                    packet_counter++;
                else{
                    serial_write_string("\n ======================", false);
                    serial_write_string(" 255 tests executed, ", false);
                    serial_write_byte(packet_failures, DEC, false);
                    serial_write_string(" tests failed,", true);
                    serial_write_string(" ======================", true);
                    packet_counter = 0;
                    packet_failures = 0;
                }
                delay_milli(50);
            }

            serial_write_string(" Test finished!\n ", false);
            serial_write_byte(packet_counter, DEC, false);
            serial_write_string(" tests executed, ", false);
            serial_write_byte(packet_failures, DEC, false);
            serial_write_string(" tests failed, ", true);
            packet_counter = 0;
            packet_failures = 0;
            delay_tick();
        }

    #elif ENV_DRONE

        bool reset = false;
        bool radio_status = false;
        static volatile uint64_t timeout_start = 0;      // first occurence of no response
        static const uint64_t timeout_max = 50000ul;     // maximum timer ticks (20µs) to clear Drone status = 1s

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
                reset = false;

                header[0] = receiving_buffer[0];
                header[1] = receiving_buffer[1];
                header[2] = receiving_buffer[2];

                switch(active_radio){
                case RADIO_NRF24:
                    serial_write_string("==> nRF24", true);
                    break;
                case RADIO_RFM95:
                    serial_write_string("==> RFM95", true);
                    break;
                }
                serial_write_string(" 0 - ", false);
                serial_write_byte(receiving_buffer[0], DEC, true);
                serial_write_string(" 1 - ", false);
                serial_write_byte(receiving_buffer[1], DEC, true);
                serial_write_string(" 2 - ", false);
                serial_write_byte(receiving_buffer[2], DEC, true);

                switch(active_radio){
                case RADIO_NRF24:
                    nrf_TXtransmit(header);
                    break;
                case RADIO_RFM95:
                    rfm_TXtransmit(header);
                    break;
                }
                active_radio = receiving_buffer[0];
                if(active_radio == 0)
                    active_radio = RADIO_NRF24;
                if(active_radio == 3) 
                    active_radio = RADIO_RFM95;
                active_channel = receiving_buffer[2];
                switch(active_radio){
                case RADIO_NRF24:
                    nrf_channel(active_channel);
                    break;
                case RADIO_RFM95:
                    rfm_channel(active_channel);
                    break;
                }
                serial_write_string("radio ", false);
                serial_write_byte(active_radio, DEC, true);
                serial_write_string("channel  ", false);
                serial_write_byte(active_channel, DEC, true);
            }
            else{                       // MESSAGE NOT RECEIVED
                if(timeout_start != 0){ // IN SILENCE ALREADY
                    if( (time_counter - timeout_start) >= timeout_max ){
                        timeout_start = 0;
                        active_radio = RADIO_NRF24;
                        active_channel = 0;
                        nrf_channel(0);
                        rfm_channel(0);
                        led_color(255, 0, 0);
                        delay_tick();
                        led_color(0, 0, 0);

                        if(reset == false){
                            serial_write_string("\n RESET", true);
                            reset = true;
                        }
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
