#include "main.h"

void core1Task(void* parameter){
    timer_core1_setup();

    while(1){
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1s delay
    }
    // core 1 task termination
    vTaskDelete(NULL);
}

void app_main(){
    serial_setup();
    gpios_setup();
    timer_core0_setup();

    // core 1 task creation
    xTaskCreatePinnedToCore(core1Task, "timer1Creator", 10000, NULL, 1, NULL, 1);

    while(1){
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1s delay
    }
    // core 0 task termination
    vTaskDelete(NULL);
}