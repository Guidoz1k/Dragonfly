#include "nrf24.h"

spi_device_handle_t spi_device;
uint8_t payload_size = 1;          // size of the payload in one single transmission (MAX = 32)
typedef enum {
    RX = 1,
    TX = 1,
} nrf_mode_t;

// INTERNAL SPI FUNCTIONS

uint8_t nrf_read_reg(uint8_t reg){
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

void nrf_write_reg(uint8_t reg, uint8_t data){
    uint8_t buffer[2] = {
        reg | 0x20,
        data
    };
    spi_transaction_t transaction = {
        .length = 8 + 8,
        .tx_buffer = buffer,
    };

    spi_device_polling_transmit(spi_device, &transaction);
}

bool nrf_bitread(uint8_t address, uint8_t bit){
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

void nrf_bitwrite(uint8_t address, uint8_t bit, bool value){
    uint8_t data = 0;

    data = nrf_read_reg(address);
    if(value == 1)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    nrf_write_reg(address, data);
}

void nrf_TXwrite(uint8_t *payload){
    uint8_t tx_data[33] = {0xFF};
    spi_transaction_t transaction = {
        .length = 8 + (8 * payload_size),
        .tx_buffer = tx_data,
    };
    uint8_t i = 0;

    tx_data[0] = 0b10100000;
    for(i = 0; i < payload_size; i++)
        tx_data[i + 1] = payload[i];

    spi_device_polling_transmit(spi_device, &transaction);
}

// ============ EXTERNAL SPI FUNCTIONS ============

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
    gpio_set_level(PIN_CE, 1);

    delay_milli(100);
    nrf_payload_size(payload_size);
    nrf_channel(STANDARDCH);
    nrf_write_reg(0x00, 0b01111111);    // power is on, complete CRC, no interrupt on IRQ pin, RX MODE
    nrf_write_reg(0x01, 0);             // NO AUTO ACK
    nrf_write_reg(0x02, 1);             // enable only data pipe 0
    nrf_write_reg(0x04, 0);             // disables re-transmits
    nrf_write_reg(0x06, 0b00100110);    // RF data rate of 250kbps, maximum TX power
    nrf_write_reg(0x07, 0b01111110);    // clear status
    nrf_TXflush();
    nrf_RXflush();
    delay_milli(100);
}

void nrf_TXflush(void){
    uint8_t command = 0b11100001;
    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &command,
    };

    spi_device_polling_transmit(spi_device, &transaction);
}

void nrf_RXflush(void){
    uint8_t command = 0b11100010;
    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &command,
    };

    spi_device_polling_transmit(spi_device, &transaction);
}

void nrf_RXread(uint8_t *payload){
    uint8_t tx_data[33] = {0xFF};
    spi_transaction_t transaction = {
        .length = 8 + (8 * payload_size),
        .tx_buffer = tx_data,
        .rx_buffer = payload,
    };

    tx_data[0] = 0b01100001;
    spi_device_polling_transmit(spi_device, &transaction);
}

void nrf_dump11reg(uint8_t *reg_p){
    uint8_t i;

    for(i = 0; i <= 0x07; i++)
        *(reg_p++) = nrf_read_reg(i);
    *(reg_p++) = nrf_read_reg(0x09);
    *(reg_p++) = nrf_read_reg(0x11);
    *(reg_p++) = nrf_read_reg(0x17);
}

void nrf_channel(uint8_t channel){
    nrf_write_reg(0x05, channel & 0b01111111);
}

void nrf_payload_size(uint8_t packets){
    payload_size = packets;
    nrf_write_reg(0x11, payload_size & 0b00111111);
}

//  checks if the RX buffer has messages available:
//        true = it has valid packets in RX buffer
//        false = empty RX buffer
bool nrf_RX_check(void){
    bool data = 0;

    if(nrf_bitread(0x17, 0) == 1)
        data = false;
    else
        data = true;

    return data;
}

bool nrf_RPD_check(void){
    bool data = 0;

    delay_micro(170);
    if(nrf_read_reg(0x09))
        data = true;
    else
        data = false;

    return data;
}

void nrf_mode_standby(void){
    gpio_set_level(PIN_CE, 0);
    nrf_bitwrite(0x00, 0, 1);
}

void nrf_mode_activeRX(void){
    gpio_set_level(PIN_CE, 1);
    nrf_bitwrite(0x00, 0, 1);
}

/*void nrf_TX_mode(void){

    //nrf_TXflush();
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
    //nrf_TXflush();

    nrf_spi_bitwrite(0x07, 4, 1); // clear MAX_RT flag
}
*/

/*void nrf_TX_transmit(uint8_t *buffer){
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
*/

/*void nrf_TX_chirp(uint data){

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
