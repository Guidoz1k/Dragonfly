#include "main.h"

#ifdef ENV_BENCHTEST
    #define BENCHTESTING true
#else
    #define BENCHTESTING false
#endif

#ifdef ENV_BASE
    const bool is_base = true;
#else
    const bool is_base = false;
#endif

void core1Task(void* parameter){ // RUNS ON CORE 1
    timer_core1_setup(is_base);

    #ifdef ENV_BASE
        task_core1BASE();
    #elif ENV_DRONE
        task_core1DRONE();
    #endif

    // core 1 task termination
    vTaskDelete(NULL);
}

void app_main(){ // RUNS ON CORE 0
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