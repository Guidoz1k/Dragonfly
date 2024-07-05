#include "nrf24.h"

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <driver/gpio.h>
#include <hal/spi_types.h>
#include <driver/spi_master.h>
#include <esp_err.h>

// ========== INTERNAL LIBRARIES ==========

#include "delay.h"

// ========== DEFINITIONS ==========

#define SPI_CH      SPI2_HOST
#define PIN_MOSI    9
#define PIN_MISO    10
#define PIN_CLK     11
#define PIN_CS      12
#define PIN_CE      13

#define STANDARDCH  0x3F

// ========== GLOBAL VARIABLES ==========

// esp_err variable
static const char *TAG = "nRF24L01+";

spi_device_handle_t spi_device;
/*
    350µs is the maximum allowed time to transmit a single byte
    t(x) = 130 us (PLL) + (1B Preample + 3B address + xB payload + 1B CRC)*8/ 0.25MBIT = 130 + 192 us (x = 1) = 322 us.
    the ammount of bytes per payload determines the transmission time therefore the payload setting functions also sets tx_time
*/
uint16_t tx_time = 0;
uint8_t payload_size = 1;          // size of the payload in one single transmission (MAX = 32)

// ============ INTERNAL FUNCTIONS ============

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

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
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

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
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

    tx_data[0] = 0b10100000;    // write in TX buffer command
    for(i = 0; i < payload_size; i++)
        tx_data[i + 1] = payload[i];
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
}

void nrf_RXread(uint8_t *payload){
    uint8_t tx_data[33] = {0xFF};
    uint8_t rx_data[33] = {0};
    spi_transaction_t transaction = {
        .length = 8 + (8 * payload_size),
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    uint8_t i = 0;

    tx_data[0] = 0b01100001;    // read RX buffer command
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
    for(i = 0; i < payload_size; i++)
        payload[i] = rx_data[i + 1];
}

void nrf_TXflush(void){
    uint8_t command = 0b11100001;
    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &command,
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
}

void nrf_RXflush(void){
    uint8_t command = 0b11100010;
    spi_transaction_t transaction = {
        .length = 8,
        .tx_buffer = &command,
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
}

// ============ EXTERNAL SPI FUNCTIONS ============

// Mandatory setup initialization function
void nrf_setup(bool test){
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
        .pin_bit_mask = (uint64_t)1 << PIN_CE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_CH, &buscfg, SPI_DMA_CH_AUTO));
    // Attach the radio to the SPI bus
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_CH, &devcfg, &spi_device));

    // GPIO config commands
    ESP_ERROR_CHECK(gpio_config(&outputs));
    ESP_ERROR_CHECK(gpio_set_level(PIN_CE, 1));

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

    ESP_LOGI(TAG, "nRF24L01+ initialized");
}

/* old debugging function
void nrf_dump11reg(uint8_t *reg_p){
    uint8_t i;

    for(i = 0; i <= 0x07; i++)
        *(reg_p++) = nrf_read_reg(i);
    *(reg_p++) = nrf_read_reg(0x09);
    *(reg_p++) = nrf_read_reg(0x11);
    *(reg_p++) = nrf_read_reg(0x17);
}
*/

// Change the channel from 0 to 127
void nrf_channel(uint8_t channel){
    nrf_write_reg(0x05, channel & 0b01111111);
}

// Change the packet size and configure the correct time for TX to transmit
void nrf_payload_size(uint8_t packets){
    payload_size = packets;

/*
    t(x) = 130 us (PLL) + (1B Preample + 3B address + xB payload + 1B CRC)*8/ 0.25MBIT + 20 us for tolerance
    t(x) = 130 + (5 + x) * 32 + 20
    t(x) = 340 + 5 * x
*/
    tx_time = 340 + 5 * packets;

    nrf_write_reg(0x11, payload_size & 0b00111111);
}

// Check the RPD register
bool nrf_RPD_check(void){
    bool data = 0;

    delay_micro(170);
    if(nrf_read_reg(0x09))
        data = true;
    else
        data = false;

    return data;
}

// Sets the transceiver in standby mode
void nrf_mode_standby(void){
    ESP_ERROR_CHECK(gpio_set_level(PIN_CE, 0));
    nrf_bitwrite(0x00, 0, 1);
}

// Sets the transceiver back in RX mode
void nrf_mode_activeRX(void){
    ESP_ERROR_CHECK(gpio_set_level(PIN_CE, 1));
    nrf_bitwrite(0x00, 0, 1);
}

// Returns true if received valid packet, false if not
bool nrf_RXreceive(uint8_t *payload){
    bool message_present = false;

    if(nrf_bitread(0x07, 6) == true){   // check if RX_DS flag bit was set (Data Ready RX FIFO)
        nrf_RXread(payload);            // passes the pointer on to the RX buffer read function
        message_present = true;         // returns true
        nrf_RXflush();                  // flushes the buffer
        nrf_bitwrite(0x07, 6, 1);       // clear flag
    }

    return message_present;
}

// Transmits packet and returns to RX mode
bool nrf_TXtransmit(uint8_t *payload){
    bool success = false;

    nrf_TXflush();                  // flushes TX buffer
    nrf_bitwrite(0x07, 4, 1);       // clear MAX_RT flag bit on status register
    nrf_TXwrite(payload);           // writes on TX buffer the payload

    ESP_ERROR_CHECK(gpio_set_level(PIN_CE, 0));     // disables radio so it can change its mode
    nrf_bitwrite(0x00, 0, 0);                       // changes mode to TX
    ESP_ERROR_CHECK(gpio_set_level(PIN_CE, 1));     // enables radio again to transmit
    delay_micro(tx_time);                           // wait time enough for radio to transmit

    //while(nrf_bitread(0x07, 5) != 1);

    ESP_ERROR_CHECK(gpio_set_level(PIN_CE, 0));     // disables radio so it can change its mode
    nrf_bitwrite(0x00, 0, 1);                       // changes mode to RX
    ESP_ERROR_CHECK(gpio_set_level(PIN_CE, 1));     // enables radio in RX mode

    if(nrf_bitread(0x07, 5) == 1){  // check if TX_DS flag bit was set (Data Sent TX FIFO)
        nrf_bitwrite(0x07, 5, 1);   // clears bit flag
        success = true;             // returns true
    }

    return success;
}
