#include "rfm95.h"

#include <stdio.h>
#include <driver/gpio.h>
#include <hal/spi_types.h>
#include <driver/spi_master.h>

#include "delay.h"

// ========== DEFINITIONS ==========

#define SPI_CH      SPI3_HOST
#define PIN_MOSI    4
#define PIN_MISO    5
#define PIN_CLK     6
#define PIN_CS      7
#define PIN_RESET   15
#define PIN_DIO0    16
#define PIN_DIO1    17
#define PIN_DIO2    17
#define PIN_DIO3    17
#define PIN_DIO4    17
#define PIN_DIO5    17

// ========== Global Variables ==========

spi_device_handle_t spi_device;

// INTERNAL SPI FUNCTIONS

uint8_t rfm_read_reg(uint8_t reg){
    uint8_t tx_data[2] = {
        reg & 0x7F,  // MSB clear for read
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
        reg | 0x80,  // MSB set for write
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

// ============ EXTERNAL SPI FUNCTIONS ============

// Mandatory setup initialization function
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
        .pin_bit_mask = 
            ((uint64_t)1 << PIN_RESET)
            | ((uint64_t)1 << PIN_DIO0)
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
    gpio_set_level(PIN_RESET, 1);
    gpio_set_level(PIN_DIO0, 0);
    gpio_set_level(PIN_DIO1, 0);
    gpio_set_level(PIN_DIO2, 0);
    gpio_set_level(PIN_DIO3, 0);
    gpio_set_level(PIN_DIO4, 0);
    gpio_set_level(PIN_DIO5, 0);

    // reset
    gpio_set_level(PIN_RESET, 0);
    delay_milli(10);
    gpio_set_level(PIN_RESET, 1);
    delay_milli(10);

/*  REGISTERS config

    // Put the device in standby mode
    rfm95w_write_register(0x01, 0x01);

    // Set frequency to 915MHz (example)
    rfm95w_set_frequency(915000000);

    // Configure other settings for FSK mode
    rfm95w_write_register(0x02, 0x00); // FSK modulation scheme
    rfm95w_write_register(0x03, 0x02); // Bitrate (example: 50 kbps)
    rfm95w_write_register(0x04, 0x41);
    rfm95w_write_register(0x05, 0x33); // Frequency deviation (example: 25 kHz)
    rfm95w_write_register(0x06, 0x80);
    rfm95w_write_register(0x12, 0x00); // Clear IRQ flags
    rfm95w_write_register(0x1D, 0x72); // RX BW (example)
    rfm95w_write_register(0x1E, 0xB4); // AFC BW (example)
    rfm95w_write_register(0x26, 0x0C); // Low datarate optimization off
    rfm95w_write_register(0x39, 0x34); // Preamble length (example: 8 bytes)
    rfm95w_write_register(0x3A, 0x40); // Sync word length (example: 3 bytes)
    rfm95w_write_register(0x3C, 0x91); // Packet config (variable length)
    rfm95w_write_register(0x3D, 0x01); // Payload length (example: 1 byte)
    rfm95w_write_register(0x4B, 0x09); // PLL hop (example)
    rfm95w_write_register(0x58, 0x1B); // Timer 1 (example)
    rfm95w_write_register(0x6F, 0x30); // Test DAGC (example)

    // Configure DIO mapping
    rfm95w_write_register(0x40, 0x00); // DIO0 mapping for RX and TX done
*/
}

// Change the frequency of the communication
void rfm_frequency(uint32_t freq){
    uint32_t temp_freq = (freq << 8) / 15625;

    rfm_write_reg(0x06, (temp_freq >> 16) & 0xFF);
    rfm_write_reg(0x07, (temp_freq >> 8) & 0xFF);
    rfm_write_reg(0x08, temp_freq & 0xFF);
}

// Transmits packet and returns to RX mode
bool nrf_TXtransmit(uint8_t *payload){
/*
void rfm95w_transmit(uint8_t *data, uint8_t length) {
    // Put the device in standby mode
    rfm95w_write_register(0x01, 0x01);

    // Write payload length
    rfm95w_write_register(0x38, length);

    // Write payload data
    for (int i = 0; i < length; i++) {
        rfm95w_write_register(0x00, data[i]);
    }

    // Put the device in TX mode
    rfm95w_write_register(0x01, 0x0B);
}
*/
return true;
}

/*
void app_main() {
    init_spi();
    rfm95w_reset();
    rfm95w_init();

    uint8_t message[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    rfm95w_transmit(message, sizeof(message));

    // Continuously check for received messages
    uint8_t value;
    while (1) {
        rfm95w_read_register(0x12, &value); // Read IRQ flags
        if (value & 0x40) { // RX done flag
            uint8_t length;
            rfm95w_read_register(0x13, &length); // Read RX payload length
            for (int i = 0; i < length; i++) {
                rfm95w_read_register(0x00, &value);
                printf("Received: 0x%02X\n", value);
            }
            rfm95w_write_register(0x12, 0x40); // Clear RX done flag
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
*/
