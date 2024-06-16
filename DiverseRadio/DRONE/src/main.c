#include "main.h"

void core1Task(void* parameter){
    timer_core1_setup();

    task_core1();
    // core 1 task termination
    vTaskDelete(NULL);
}

void app_main(){
    serial_setup();
    nrf_setup();
    timer_core0_setup();

    // core 1 task creation
    xTaskCreatePinnedToCore(core1Task, "timer1Creator", 10000, NULL, 1, NULL, 1);

    task_core0();
    // core 0 task termination
    vTaskDelete(NULL);
}