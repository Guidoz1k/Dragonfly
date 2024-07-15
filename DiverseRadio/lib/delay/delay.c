/*
the previous iteration of this library used the function:

    void delay_oneshot(uint16_t microseconds){
        microseconds = (microseconds & 0b1111111111100000) - 7;   // function has an overhead of 7µs and gets really unstable below 36µs
        esp_timer_start_once(oneshot_timer, microseconds);
        while(wait_complete == false);
        wait_complete = false;
    }

function has an overhead of aprox. 7µs, it gets really unstable below 36µs and if t==0, then total delay time is of aprox 14µs.
it needed a setup:

    const esp_timer_create_args_t oneshot_timer_args = {
        .callback = &oneshot_timer_callback,
        // argument specified here will be passed to timer callback function
        .arg = (void*) oneshot_timer, // i don't fucking know
        .name = "one-shot",
        .dispatch_method = ESP_TIMER_ISR,
    };
    esp_timer_create(&oneshot_timer_args, &oneshot_timer);

global variables:

    esp_timer_handle_t oneshot_timer;
    bool wait_complete = false;

and a callback function:
    static void oneshot_timer_callback(void* arg){
        wait_complete = true;
    }

and it needed CONFIG_ESP_TIMER_SUPPORTS_ISR_DISPATCH_METHOD to be set to YES in KConfig

now everything is solved by esp_timer_get_time() inside the function delay_micro(uint32_t microseconds).
the overhead fluctuates from approximately 0.75µs to 1.4µs
*/
#include "delay.h"

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_timer.h>

// ============ EXTERNAL FUNCTIONS ============

// yield the process just to slap the watchdog
void delay_tick(void){
    vTaskDelay(1);
}

// Masking of FreeRTOS delay function
void delay_milli(uint16_t period){
    vTaskDelay(pdMS_TO_TICKS(period));
}

// Pooling delay using esp timer count
void delay_micro(uint32_t microseconds){
    uint64_t start_time = esp_timer_get_time();
    while( esp_timer_get_time() < (start_time + microseconds - 1) );
}
