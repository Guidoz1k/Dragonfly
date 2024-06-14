#include "nrf24.h"

spi_device_handle_t spi_device;

// INITIAL SETUP AND SPI FUNCTIONS

void nrf_setup(void){
    // SPI variables
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_MISO,
        .mosi_io_num = PIN_MOSI,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000,     // Clock out at 10 MHz
        .mode = 0,                              // SPI mode 0
        .spics_io_num = PIN_CS,                 // CS pin
        .queue_size = 7,
    };
    // GPIO config variables
    gpio_config_t outputs = {
        .pin_bit_mask = (uint64_t)1 << PIN_CE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // Initialize the SPI bus
    spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_DISABLED); // SPI2_HOST
    // Attach the radio to the SPI bus
    spi_bus_add_device(HSPI_HOST, &devcfg, &spi_device);

    // GPIO config commands
    gpio_config(&outputs);
    gpio_set_level(PIN_CE, 0);
}

uint8_t nrf_read_reg(uint8_t reg){
    uint8_t tx_data[2] = {reg, 0xFF};
    uint8_t rx_data[2] = {0};
    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    spi_device_polling_transmit(spi_device, &transaction);

    return rx_data[1];
}

void nrf_write_reg(uint8_t reg, uint8_t data){
    uint8_t buffer[2] = {reg | 0x20, data};
    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = buffer,
    };

    spi_device_polling_transmit(spi_device, &transaction);
}

void nrf_command(uint8_t command){ //////////////// NOT DONE
    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &command,
    };

    spi_device_polling_transmit(spi_device, &transaction);
}

bool spi_bitread(uint8_t address, uint8_t bit){
    uint8_t data = 0;
    bool bitread = false;

    data = nrf_read_reg(address);
    data &= 1 << bit;
    if(data != 0)
        bitread = true;
    else
    bitread = false;

    return bitread;
}

void spi_bitwrite(uint8_t address, uint8_t bit, bool value){
    uint8_t data = 0;

    data = nrf_read_reg(address);
    if(value == 1)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    nrf_write_reg(address, data);
}

// ====== EXTERNAL FUNCTIONALITIES ======
/*
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


//  checks if the RX buffer has messages available:
//        true = it has valid packets in RX buffer
//        false = empty RX buffer

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


// returns values from all nRF24 radio's 26 registers

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

void nrf_addressP0(uint8_t *buffer){
    const uint8_t addressRX = 0x0A;
    const uint8_t addressTX = 0x10;
    uint8_t aux;


    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    SPI.transfer(addressRX | (1 << 5));
    SPI.transfer(buffer[0]);
    SPI.transfer(buffer[1]);
    SPI.transfer(buffer[2]);
    SPI.transfer(buffer[3]);
    SPI.transfer(buffer[4]);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();

    SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
    digitalWriteFast(CS_NRF, LOW);

    SPI.transfer(addressTX | (1 << 5));
    SPI.transfer(buffer[0]);
    SPI.transfer(buffer[1]);
    SPI.transfer(buffer[2]);
    SPI.transfer(buffer[3]);
    SPI.transfer(buffer[4]);

    digitalWriteFast(CS_NRF, HIGH);
    SPI.endTransaction();
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
    nrf_spi_bitwrite(0x07, 4, 1); // clear MAX_RT flag

    //digitalWriteFast(CE_NRF, LOW);
        delayMicroseconds(1000);
    nrf_spi_write(0x00, 0b01111110); // TX mode
        delayMicroseconds(1000);
    //digitalWriteFast(CE_NRF, HIGH);
        delayMicroseconds(1000);
    //digitalWriteFast(CE_NRF, LOW);
        delayMicroseconds(1000);
    //delayMicroseconds(130); // radio settling time

    //while(nrf_spi_bitread(0x17, 4) != 1);
    nrf_RX_mode();
    //nrf_flushTX();

    nrf_spi_bitwrite(0x07, 4, 1); // clear MAX_RT flag
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
*/
