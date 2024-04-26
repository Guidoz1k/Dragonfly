#include "diverseSPI.h"

uint8_t spi_nrf(){
    uint8_t error = 0;

    digitalWriteFast(CS_NRF, LOW);

    // code spi

    digitalWriteFast(CS_NRF, HIGH);

    return error;
}

uint8_t spi_rfm(){
    uint8_t error = 0;

    digitalWriteFast(CS_RFM, LOW);

    // code spi

    digitalWriteFast(CS_RFM, HIGH);

    return error;
}

void nrf_setup(void){
    // code
}

void rfm_setup(void){
    // code
}

void spi_setup(void){
    pinMode(SPI_CK, OUTPUT);
    pinMode(SPI_MI, OUTPUT);
    pinMode(SPI_MO, OUTPUT);

    pinMode(CS_NRF, OUTPUT);
    digitalWrite(CS_NRF, HIGH);
    pinMode(CS_RFM, OUTPUT);
    digitalWrite(CS_RFM, HIGH);
    pinMode(CE_NRF, OUTPUT);
    digitalWrite(CE_NRF, LOW);

    SPI.begin();
}

void diverse_setup(void){
    spi_setup();
    nrf_setup();
    rfm_setup();
}
