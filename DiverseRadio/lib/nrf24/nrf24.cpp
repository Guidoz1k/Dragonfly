#include <nrf24.h>

uint8_t nrf_spi_read(uint8_t address){
    uint8_t data;

    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    SPI.transfer(address);
    data = SPI.transfer(0);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();

    return data;
}

uint8_t nrf_spi_write(uint8_t address, uint8_t data){
    uint8_t status;

    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    status = SPI.transfer(address | (1 << 5)); // to write to registers, add 32 to the register
    SPI.transfer(data);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();

    return status;
}

uint8_t nrf_spi_command(uint8_t data){
    uint8_t status;

    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    status = SPI.transfer(data);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();

    return status;
}

boolean nrf_spi_bitread(uint8_t address, uint8_t bit){
    uint8_t data;
    boolean bitread;

    data = nrf_spi_read(address);
    data &= 1 << bit;
    if(data != 0)
        bitread = true;
    else
    bitread = false;

    return bitread;
}

void nrf_spi_bitwrite(uint8_t address, uint8_t bit, boolean value){
    uint8_t data;

    data = nrf_spi_read(address);
    if(value == 1)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    nrf_spi_write(address, data);
}

uint8_t nrf_channel(uint8_t channel){
    uint8_t data;

    data = nrf_spi_write(0x05, channel && 0b01111111);

    return data;
}

uint8_t nrf_flushTX(void){
    uint8_t data;

    data = nrf_spi_command(0b11100001);

    return data;
}

uint8_t nrf_flushRX(void){
    uint8_t data;

    data = nrf_spi_command(0b11100010);

    return data;
}

boolean nrf_check_RX_buffer(void){
    boolean data;

    if(nrf_spi_bitread(0x17, 0))
        data = false;
    else
        data = true;

    return data;
}

boolean nrf_check_RX_power(void){ // TODO
    boolean data;

    // TODO: should spend 170us in RX mode
    if(nrf_spi_read(0x09))
        data = true;
    else
        data = false;

    return data;
}

void nrf_setup(void){ // TODO
    pinMode(SPI_CK, OUTPUT);
    pinMode(SPI_MI, INPUT);
    pinMode(SPI_MO, OUTPUT);

    pinMode(CS_NRF, OUTPUT);
    digitalWrite(CS_NRF, HIGH);
    pinMode(CE_NRF, OUTPUT);
    digitalWrite(CE_NRF, LOW);

    SPI.begin();

    nrf_spi_write(0x00, 0b01111111);    // no usage of IRQ, 2 bytes CRC on, power on in RX mode
    nrf_spi_write(0x01, 0b00000000);    // no shockburst
    nrf_spi_write(0x02, 0b00000001);    // enable data pipe 0
    nrf_spi_write(0x04, 0b00000000);    // no auto retransmission
    nrf_spi_write(0x06, 0b00100110);    // maximum power with no continuous carrier
    nrf_spi_write(0x11, PACKETS); // number of packets expected to be received by transmission
    nrf_flushTX();
    nrf_flushRX();

    Serial.begin(115200);
    delay(1000);
    Serial.println(nrf_spi_read(0x01));
    Serial.println(nrf_spi_read(0x02));
    Serial.println(nrf_spi_read(0x04));
    Serial.println(nrf_spi_read(0x06));
    Serial.println(nrf_spi_read(0x11));
    while(1);
}
