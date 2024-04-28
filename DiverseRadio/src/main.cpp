#include <main.h>

void init(void){
    Serial.begin(115200);

    if(nrf_setup(true) == true){
        Serial.println("nRF24L01+ radio boot successful");
        if(rfm_setup() == true){
            Serial.println("RFM95W radio boot successful");
        }
        else{
            Serial.println("RFM95W radio boot ERROR");
            while(1);
        }
    }
    else{
        Serial.println("nRF24L01+ radio boot ERROR");
        while(1);
    }
}

int main(){
    init();

    while(1);
}
