#include "main.h"

// environment test for power transmission setup
#ifdef ENV_BENCHTEST
    #define BENCHTESTING true
#else
    #define BENCHTESTING false
#endif

// environment test for BASE or DRONE system
#ifdef ENV_BASE
    const bool is_base = true;
#else
    const bool is_base = false;
#endif

 // RUNS ON CORE 1
void core1Task(void* parameter){
    timer_core1_setup(is_base);

    #ifdef ENV_BASE
        task_core1BASE();
    #elif ENV_DRONE
        task_core1DRONE();
    #endif

    // core 1 task termination
    vTaskDelete(NULL);
}

 // RUNS ON CORE 0
void app_main(){
    delay_milli(1000);
    led_setup();
    serial_setup();
    nrf_setup(BENCHTESTING);
    timer_core0_setup(is_base);

    // core 1 task creation
    xTaskCreatePinnedToCore(core1Task, "timer1Creator", 10000, NULL, 1, NULL, 1);

    #ifdef ENV_BASE
        task_core0BASE();
    #elif ENV_DRONE
        task_core0DRONE();
    #endif

    // core 0 task termination
    vTaskDelete(NULL);
}
