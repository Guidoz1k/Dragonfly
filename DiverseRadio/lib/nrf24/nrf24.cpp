#include <nrf24.h>

uint8_t protocol_packets = 1;

// ====== INTERNAL SPI FUNCTIONS ======

uint8_t nrf_spi_read(uint8_t address){
    uint8_t data = 0;

    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    SPI.transfer(address);
    data = SPI.transfer(0);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();

    return data;
}

uint8_t nrf_spi_write(uint8_t address, uint8_t data){
    uint8_t status = 0;

    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    status = SPI.transfer(address | (1 << 5)); // to write to registers, add 32 to the register
    SPI.transfer(data);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();

    return status;
}

uint8_t nrf_spi_command(uint8_t data){
    uint8_t status = 0;

    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    status = SPI.transfer(data);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();

    return status;
}

bool nrf_spi_bitread(uint8_t address, uint8_t bit){
    uint8_t data = 0;
    bool bitread = false;

    data = nrf_spi_read(address);
    data &= 1 << bit;
    if(data != 0)
        bitread = true;
    else
    bitread = false;

    return bitread;
}

void nrf_spi_bitwrite(uint8_t address, uint8_t bit, boolean value){
    uint8_t data = 0;

    data = nrf_spi_read(address);
    if(value == 1)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    nrf_spi_write(address, data);
}

// ====== EXTERNAL FUNCTIONALITIES ======

uint8_t nrf_channel(uint8_t channel){
    uint8_t data = 0;

    data = nrf_spi_write(0x05, channel & 0b01111111);

    return data;
}

uint8_t nrf_packets(uint8_t packets){
    uint8_t data = 0;

    protocol_packets = packets;
    data = nrf_spi_write(0x11, protocol_packets & 0b00111111);

    return data;
}

uint8_t nrf_flushTX(void){
    uint8_t data = 0;

    data = nrf_spi_command(0b11100001);

    return data;
}

uint8_t nrf_flushRX(void){
    uint8_t data = 0;

    data = nrf_spi_command(0b11100010);

    return data;
}

/*  checks if the RX buffer has messages available:
        true = it has valid packets in RX buffer
        false = empty RX buffer
*/
bool nrf_check_RX_buffer(void){
    bool data = 0;

    if(nrf_spi_bitread(0x17, 0) == 1)
        data = false;
    else
        data = true;

    return data;
}

bool nrf_check_RPD(void){
    bool data = 0;

    delayMicroseconds(170);
    if(nrf_spi_read(0x09))
        data = true;
    else
        data = false;

    return data;
}

/*
returns values from all nRF24 radio's 26 registers
*/
void nrf_registers(uint8_t *buffer){
    *(buffer++) = nrf_spi_read(0x00);
    *(buffer++) = nrf_spi_read(0x01);
    *(buffer++) = nrf_spi_read(0x02);
    *(buffer++) = nrf_spi_read(0x03);
    *(buffer++) = nrf_spi_read(0x04);
    *(buffer++) = nrf_spi_read(0x05);
    *(buffer++) = nrf_spi_read(0x06);
    *(buffer++) = nrf_spi_read(0x07);
    *(buffer++) = nrf_spi_read(0x08);
    *(buffer++) = nrf_spi_read(0x09);
    *(buffer++) = nrf_spi_read(0x0A);
    *(buffer++) = nrf_spi_read(0x0B);
    *(buffer++) = nrf_spi_read(0x0C);
    *(buffer++) = nrf_spi_read(0x0D);
    *(buffer++) = nrf_spi_read(0x0E);
    *(buffer++) = nrf_spi_read(0x0F);
    *(buffer++) = nrf_spi_read(0x10);
    *(buffer++) = nrf_spi_read(0x11);
    *(buffer++) = nrf_spi_read(0x12);
    *(buffer++) = nrf_spi_read(0x13);
    *(buffer++) = nrf_spi_read(0x14);
    *(buffer++) = nrf_spi_read(0x15);
    *(buffer++) = nrf_spi_read(0x16);
    *(buffer++) = nrf_spi_read(0x17);
    *(buffer++) = nrf_spi_read(0x1C);
    *(buffer++) = nrf_spi_read(0x1D);
}

// RADIO OPERATIONS

void nrf_RX_mode(void){
    nrf_spi_write(0x00, 0b01111111); // RX mode
    digitalWrite(CE_NRF, HIGH);
    delayMicroseconds(130); // radio settling time
}

void nrf_RX_read(uint8_t *buffer){
    uint8_t aux;

    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    SPI.transfer(0b01100001); // read RX payload
    for(aux=0; aux < protocol_packets; aux++)
        *(buffer++) = SPI.transfer(0);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();
}

void nrf_standby_mode(void){
    digitalWrite(CE_NRF, LOW);
    nrf_spi_bitwrite(0x00, 0, 1);
}

void nrf_TX_mode(void){

    //nrf_flushTX();
    //nrf_spi_bitwrite(0x07, 4, 1); // clear MAX_RT flag

    digitalWriteFast(CE_NRF, LOW);
        delayMicroseconds(10);
    nrf_spi_write(0x00, 0b01111110); // TX mode
        delayMicroseconds(10);
    digitalWriteFast(CE_NRF, HIGH);
        delayMicroseconds(10);
    //delayMicroseconds(130); // radio settling time

    //while(nrf_spi_bitread(0x17, 4) != 1);
    nrf_RX_mode();
    //nrf_flushTX();

    //nrf_spi_bitwrite(0x07, 4, 1); // clear MAX_RT flag
}

void nrf_TX_transmit(uint8_t *buffer){
    uint8_t aux;

    // begins SPI transfer of TX payload
    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    SPI.transfer(0b10100000); // write TX payload
    for(aux=0; aux < protocol_packets; aux++)
        SPI.transfer(*(buffer++));

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();
    // the end of SPI transfer of TX payload

    nrf_TX_mode();
}

void nrf_TX_chirp(uint data){

    // begins SPI transfer of TX payload
    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    SPI.transfer(0b10100000); // write TX payload
    SPI.transfer(data);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();
    // the end of SPI transfer of TX payload

    nrf_TX_mode();
}

// INITIAL SETUP

bool nrf_setup(void){
    bool test = false;

    pinMode(SPI_CK, OUTPUT);
    pinMode(SPI_MI, INPUT);
    pinMode(SPI_MO, OUTPUT);

    pinMode(CS_NRF, OUTPUT);
    digitalWrite(CS_NRF, HIGH);
    pinMode(CE_NRF, OUTPUT);
    digitalWrite(CE_NRF, HIGH); // starts in RX mode

    SPI.begin();

    nrf_spi_write(0x00, 0b01111111);    // no usage of IRQ, 2 bytes CRC on, power on in RX mode
    nrf_spi_write(0x01, 0b00000000);    // no shockburst
    nrf_spi_write(0x02, 0b00000001);    // enable data pipe 0
    nrf_spi_write(0x04, 0b00000000);    // no auto retransmission
    nrf_spi_write(0x06, 0b00100110);    // maximum power with no continuous carrier
    //nrf_spi_write(0x06, 0b00100000);    // minimum power with no continuous carrier

    nrf_flushTX();
    nrf_flushRX();
    nrf_channel(0x69);
    nrf_packets(protocol_packets); // standard value

    nrf_spi_write(0x07, 0x70);  // resets STATUS flags
    if(nrf_spi_read(0x06) == 0b00100110){
        test = true;
        delayMicroseconds(1500); // cool radio settling time
    }
    
    //nrf_RX_mode();
    return test;
}

