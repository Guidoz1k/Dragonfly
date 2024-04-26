#include <nrf24.h>

uint8_t spi_nrf(void){
    uint8_t error = 0;

    digitalWriteFast(CS_NRF, LOW);

    // code spi

    digitalWriteFast(CS_NRF, HIGH);

    return error;
}

void nrf_setup(void){
    pinMode(SPI_CK, OUTPUT);
    pinMode(SPI_MI, OUTPUT);
    pinMode(SPI_MO, OUTPUT);

    pinMode(CS_NRF, OUTPUT);
    digitalWrite(CS_NRF, HIGH);
    pinMode(CE_NRF, OUTPUT);
    digitalWrite(CE_NRF, LOW);

    SPI.begin();
}