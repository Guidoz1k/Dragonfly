#include "rfm95.h"

spi_device_handle_t spi_device;

void rfm_setup(void){
    // SPI variables
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_MISO,
        .mosi_io_num = PIN_MOSI,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4092,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,     // Clock out at 10 MHz
        .mode = 0,                              // SPI mode 0
        .spics_io_num = PIN_CS,                 // CS pin
        .queue_size = 7,
    };
    // GPIO config variables
    gpio_config_t outputs = {
        .pin_bit_mask = ((uint64_t)1 << PIN_DIO0)
            | ((uint64_t)1 << PIN_DIO1)
            | ((uint64_t)1 << PIN_DIO2)
            | ((uint64_t)1 << PIN_DIO3)
            | ((uint64_t)1 << PIN_DIO4)
            | ((uint64_t)1 << PIN_DIO5),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // Initialize the SPI bus
    spi_bus_initialize(SPI_CH, &buscfg, SPI_DMA_CH_AUTO);
    // Attach the radio to the SPI bus
    spi_bus_add_device(SPI_CH, &devcfg, &spi_device);

    // GPIO config commands
    gpio_config(&outputs);
    gpio_set_level(PIN_DIO0, 0);
    gpio_set_level(PIN_DIO1, 0);
    gpio_set_level(PIN_DIO2, 0);
    gpio_set_level(PIN_DIO3, 0);
    gpio_set_level(PIN_DIO4, 0);
    gpio_set_level(PIN_DIO5, 0);

/*  REGISTERS config
    delay_milli(100);
    nrf_payload_size(payload_size);
    nrf_channel(STANDARDCH);
    nrf_write_reg(0x00, 0b01111111);    // power is on, complete CRC, no interrupt on IRQ pin, RX MODE
    nrf_write_reg(0x01, 0);             // NO AUTO ACK
    nrf_write_reg(0x02, 1);             // enable only data pipe 0
    nrf_write_reg(0x03, 3);             // 3 bytes address
    nrf_write_reg(0x04, 0);             // disables re-transmits

    if(test == true)
        nrf_write_reg(0x06, 0b00100000);    // RF data rate of 250kbps, minimum TX power for bench test
    else
        nrf_write_reg(0x06, 0b00100110);    // RF data rate of 250kbps, maximum TX power for field test

    nrf_write_reg(0x07, 0b01111110);    // clear status
    nrf_TXflush();
    nrf_RXflush();
    delay_milli(100);
*/
}

uint8_t rfm_read_reg(uint8_t reg){
    uint8_t tx_data[2] = {
        reg,
        0xFF
    };
    uint8_t rx_data[2] = {0};
    spi_transaction_t transaction = {
        .length = 8 + 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    spi_device_polling_transmit(spi_device, &transaction);
    return rx_data[1];
}

void rfm_write_reg(uint8_t reg, uint8_t data){
    uint8_t buffer[2] = {
        reg, // | 0x20,
        data
    };
    spi_transaction_t transaction = {
        .length = 8 + 8,
        .tx_buffer = buffer,
    };

    spi_device_polling_transmit(spi_device, &transaction);
}

bool rfm_bitread(uint8_t address, uint8_t bit){
    uint8_t data = 0;
    bool bitread = false;

    data = rfm_read_reg(address);
    data &= 1 << bit;
    if(data != 0)
        bitread = true;
    else
    bitread = false;

    return bitread;
}

void nrf_bitwrite(uint8_t address, uint8_t bit, bool value){
    uint8_t data = 0;

    data = rfm_read_reg(address);
    if(value == 1)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    rfm_write_reg(address, data);
}
