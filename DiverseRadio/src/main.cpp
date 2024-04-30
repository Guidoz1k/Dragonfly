#include <main.h>

void init(void){
    Serial.begin(115200);

    if(nrf_setup() == true){
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
    uint8_t buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    pinMode(TESTLED, OUTPUT);
    pinMode(TESTBUT, INPUT);

    init();

    if(DEVICE == 0){
        delay(3000);
        while(1){
            while(digitalReadFast(TESTBUT) == LOW){
                delay(100);
                digitalWriteFast(TESTLED, LOW);
                delay(100);
                digitalWriteFast(TESTLED, HIGH);
            }
            nrf_TX_chirp(0x01);
            delay(1000);
        }
    }
    else{
        while(1){
            digitalWriteFast(TESTLED, HIGH);
            nrf_registers(buffer);
            //Serial.print("0x00 = "); Serial.println(buffer[0]);
            //Serial.print("0x01 = "); Serial.println(buffer[1]);
            //Serial.print("0x02 = "); Serial.println(buffer[2]);
            //Serial.print("0x04 = "); Serial.println(buffer[3]);
            //Serial.print("0x06 = "); Serial.println(buffer[4]);
            Serial.print("0x17 = "); Serial.println(buffer[5]);
            delay(100);
            digitalWriteFast(TESTLED, LOW);
            //nrf_RX_read(buffer);
            //Serial.print("  RX = "); Serial.println(nrf_check_RX_buffer());
            //Serial.println("");
            delay(900);
        }
    }

}
