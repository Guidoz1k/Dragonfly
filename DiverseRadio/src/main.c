#include "main.h"

typedef enum{
    RED,
    GREEN,
    BLUE
} led_mode_t;

void app_main(){
    uint8_t pwm_r = 0;
    uint8_t pwm_g = 0;
    uint8_t pwm_b = 0;
    led_mode_t led_mode = RED;

    led_init();
    for(pwm_b = 0; pwm_b < 255; pwm_b++){
        led_color(pwm_r, pwm_g, pwm_b);
        vTaskDelay(pdMS_TO_TICKS(DELAY_TIME));
    }
    while(1){
        switch(led_mode){
        case RED:
            pwm_r++;
            pwm_b--;
            if(pwm_r == 255)
                led_mode = GREEN;
            break;
        case GREEN:
            pwm_g++;
            pwm_r--;
            if(pwm_g == 255)
                led_mode = BLUE;
            break;
        case BLUE:
            pwm_b++;
            pwm_g--;
            if(pwm_b == 255)
                led_mode = RED;
            break;
        }

        led_color(pwm_r, pwm_g, pwm_b);
        vTaskDelay(pdMS_TO_TICKS(DELAY_TIME));
    }
}

