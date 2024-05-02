#include <Arduino.h>

#include <nrf24.h>
#include <rfm95.h>

#define DEVICE 0

void init(void){
    uint8_t aux;

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

/*
    base:
    - 10 : dec print
    - 16 : hex print
    - 02 : bit print
    indexation:
    - 0 : no index
    - 1 : uses index
    - 2 : clear index and prints nothing
*/
void sprint(uint8_t data, uint8_t base,uint8_t index, bool newline){
    uint8_t aux = 0;
    uint8_t binary[8];

    if(index != 0xff){
        Serial.print("0x");
        if((index / 16) == 0)
            Serial.print("0");
        Serial.print(index, 16);
        Serial.print(": ");
        Serial.print("\t");
    }

    switch(base){
    case 10:
        Serial.print(data, 10);
        break;
    case 16:
        if((data / 16) == 0)
            Serial.print("0");
        Serial.print(data, 16);
        break;
    case 2:
        for(aux = 0; aux < 8; aux++){
            if((data % 2) == 1)
                binary[aux] = 1;
            else
                binary[aux] = 0;
            data /= 2;
        }
        for(aux = 8; aux > 0; aux--){
            if(aux == 4)
                Serial.print(".");
            Serial.print(binary[aux - 1]);
        }
        break;
    default:
        break;
    }

    if(newline == true)
        Serial.println("");
}

int main(){
    uint8_t bufferino[26];
    uint8_t buffer[26][2];
    uint8_t aux = 0, aux2 = 0;

    init();

    for(aux = 0; aux < 26; aux++){
        buffer[aux][0] = 0;
        buffer[aux][1] = aux;
    }
    buffer[24][1] = 0x1C;
    buffer[25][1] = 0x1D;

    while(1){
        for(aux2 = 0; aux2 < 3; aux2++){
                delay(1000);
                nrf_registers(bufferino);

                for(aux = 0; aux < 26; aux++)
                    if(bufferino[aux] != buffer[aux][0]){
                        Serial.print("diff! add:");
                        sprint(buffer[aux][1], 16, 0xFF, 0);
                        Serial.print(" old value: ");
                        sprint(buffer[aux][0], 2, 0xFF, 0);
                        Serial.print(" new value: ");
                        sprint(bufferino[aux], 2, 0xFF, 1);
                        buffer[aux][0] = bufferino[aux];
                    }
                nrf_RX_read(bufferino);
                if(bufferino[0] != 0){
                    Serial.print("message received: ");
                    sprint(bufferino[0], 2, 0xAA, 1);
                }
        }
        nrf_flushTX();
        Serial.println("TX buffer cleared");
        if(DEVICE == 0){
            nrf_TX_chirp(0xAA);
            Serial.println("TX");
        }
    }
}
