#include "spi.h"

spi_device_handle_t spi;

void spi_init(void){
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_MISO,
        .mosi_io_num = PIN_MOSI,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,     // Clock out at 10 MHz
        .mode = 0,                              // SPI mode 0
        .spics_io_num = PIN_CS,                 // CS pin
    };
    // Initialize the SPI bus
    spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_DISABLED); // SPI2_HOST
    // Attach the radio to the SPI bus
    spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
}

void spi_command(uint8_t command){

}

void spi_write(uint8_t reg, uint8_t data){
    uint8_t buffer[2] = {NRF_CMD_W_REGISTER | reg, data};
    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = buffer,
    };

    spi_device_polling_transmit(spi, &transaction);
}

uint8_t spi_read(uint8_t reg){
    uint8_t tx_data[2] = {NRF_CMD_R_REGISTER | reg, 0xFF};
    uint8_t rx_data[2] = {0};
    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    spi_device_polling_transmit(spi, &transaction);

    return rx_data[1];
}

bool spi_bitread(uint8_t address, uint8_t bit){
    uint8_t data = 0;
    bool bitread = false;

    data = spi_read(address);
    data &= 1 << bit;
    if(data != 0)
        bitread = true;
    else
    bitread = false;

    return bitread;
}

void spi_bitwrite(uint8_t address, uint8_t bit, bool value){
    uint8_t data = 0;

    data = spi_read(address);
    if(value == 1)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    spi_write(address, data);
}