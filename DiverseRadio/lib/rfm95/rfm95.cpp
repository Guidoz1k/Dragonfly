#include <rfm95.h>

uint8_t spi_rfm(void){
    uint8_t error = 0;

    digitalWriteFast(CS_RFM, LOW);

    // code spi

    digitalWriteFast(CS_RFM, HIGH);

    return error;
}

bool rfm_setup(void){
    //bool test = 0;

    pinMode(SPI_CK, OUTPUT);
    pinMode(SPI_MI, OUTPUT);
    pinMode(SPI_MO, OUTPUT);

    pinMode(CS_RFM, OUTPUT);
    digitalWrite(CS_RFM, HIGH);

    SPI.begin();

    return true;
}