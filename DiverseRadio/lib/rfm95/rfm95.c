#include "rfm95.h"

// ========== IDF LIBRARIES ==========

#include <stdio.h>
#include <driver/gpio.h>
#include <hal/spi_types.h>
#include <driver/spi_master.h>
#include <esp_err.h>
#include <esp_log.h>

// ========== INTERNAL LIBRARIES ==========

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
#define PIN_DIO2    18
#define PIN_DIO3    8
#define PIN_TEST    42

// ========== GLOBAL VARIABLES ==========

static const char *TAG = "RFM95W";   // esp_err variable

static spi_device_handle_t spi_device;  // esp_err variable

/* 780µs is the maximum allowed time to transmit a single byte
   t(x) = 130 us (PLL) + (1B Preample + 1B sync word + 1B address + xB payload + 2B CRC)*8/ 0.25MBIT = 130 + 192 us (x = 1) = 322 us.
   the ammount of bytes per payload determines the transmission time therefore the payload setting functions also sets tx_time
*/
static uint16_t tx_time = 0;
static uint8_t payload_size = 1;    // size of the payload in one single transmission (MAX = 32)

// ============ INTERNAL SPI FUNCTIONS ============

static void gpio_init_and_reset(void){
    gpio_config_t outputs = {
        .pin_bit_mask = (uint64_t)1 << PIN_RESET
            | (uint64_t)1 << PIN_TEST,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config_t inputs = {
        .pin_bit_mask = 
            ((uint64_t)1 << PIN_DIO0)
            | ((uint64_t)1 << PIN_DIO1)
            | ((uint64_t)1 << PIN_DIO2)
            | ((uint64_t)1 << PIN_DIO3),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // GPIO config commands
    ESP_ERROR_CHECK(gpio_config(&outputs));
    ESP_ERROR_CHECK(gpio_config(&inputs));
    ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 1));

    // reset
    ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 0));
    delay_milli(10);
    ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 1));
    delay_milli(10);
}

static uint8_t rfm_read_reg(uint8_t reg){
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

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
    return rx_data[1];
}

static void rfm_write_reg(uint8_t reg, uint8_t data){
    uint8_t buffer[2] = {
        reg | 0x80,  // MSB set for write
        data
    };
    spi_transaction_t transaction = {
        .length = 8 + 8,
        .tx_buffer = buffer,
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
}

static bool rfm_bitread(uint8_t address, uint8_t bit){
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

static void rfm_bitwrite(uint8_t address, uint8_t bit, bool value){
    uint8_t data = 0;

    data = rfm_read_reg(address);
    if(value == 1)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    rfm_write_reg(address, data);
}

static void rfm_TXwrite(uint8_t *payload){
    uint8_t tx_data[33] = {0};
    spi_transaction_t transaction = {
        .length = 8 + (8 * payload_size),
        .tx_buffer = tx_data,
    };
    uint8_t i = 0;

    tx_data[0] = 0x80;  // MSB set for write
    for(i = 0; i < payload_size; i++)
        tx_data[i + 1] = payload[i];
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
}

static void rfm_RXread(uint8_t *payload){
    uint8_t tx_data[33] = {0};
    uint8_t rx_data[33] = {0};
    spi_transaction_t transaction = {
        .length = 8 + (8 * payload_size),
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    uint8_t i = 0;

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &transaction));
    for(i = 0; i < payload_size; i++)
        payload[i] = rx_data[i + 1];
}

static void rfm_bitrate(uint32_t bitrate){
// bit rate = FXOSC / bit rate reg
// bit rate reg = 32000000 / bit rate
    uint32_t temp_br;   // bit rate

    if( (bitrate > 0) && (bitrate <= 300000ul) ){
        temp_br = 32000000ul / bitrate;

        rfm_write_reg(0x02, (temp_br >> 8) & 0xFF);
        rfm_write_reg(0x03, temp_br & 0xFF);

        delay_micro(150);
    }
}

static void rfm_frequency_carrier(uint32_t freq){
// carrier frequency = step * freq_reg
// step = 15625 / 256
// freq_reg = carrier frequency / step
    uint64_t temp_freq;

    if( (freq >= 902000000ul) && (freq <= 928000000ul) ){
        temp_freq = (uint64_t)((uint64_t)freq << 8) / 15625;

        rfm_write_reg(0x06, (temp_freq >> 16) & 0xFF);
        rfm_write_reg(0x07, (temp_freq >> 8) & 0xFF);
        rfm_write_reg(0x08, temp_freq & 0xFF);

        delay_micro(150);
    }

}

static void rfm_frequency_deviation(uint32_t freq){
// frequency deviation = step * freq_reg
// step = 15625 / 256
// freq_reg = frequency deviation / step
    uint64_t temp_freq;

    if( (freq >= 600) && (freq <= 200000ul) ){
        temp_freq = (uint64_t)((uint64_t)freq << 8) / 15625;

        rfm_write_reg(0x04, (temp_freq >> 8) & 0x3F); 
        rfm_write_reg(0x05, temp_freq & 0xFF);
        
        delay_micro(100);
    }
}

// ============ EXTERNAL SPI FUNCTIONS ============

// Mandatory setup initialization function
void rfm_setup(void){
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

    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_CH, &buscfg, SPI_DMA_CH_AUTO));
    // Attach the radio to the SPI bus
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_CH, &devcfg, &spi_device));

    gpio_init_and_reset(); // gpio configuration and radio reset

    rfm_write_reg(0x01, 0b00000001);    // FSK mode, high frequency, radio is in STAND BY
    rfm_frequency_carrier(902 *1000 * 1000 + 200 * 1000);
    rfm_bitwrite(0x24, 3, 1);           // calibrates chip after frequency change to HF

    // To ensure correct modulation: Fdev + BR/2 <= 250kHz
    // Beta = 2 * Fdev/BR
    // 0.5 <= Beta <= 10
    rfm_frequency_deviation(150 * 1000);
    rfm_bitrate(50 * 1000);    // bit rate
    rfm_write_reg(0x12, 0x01);  // RX Bw = 250kHz

    rfm_write_reg(0x1F, 0xAA);  // 2 preamble bytes to detect
    rfm_write_reg(0x26, 0x03);  // 3 preamble bytes sent

    rfm_write_reg(0x4D, 0x07);  // PA boost
    rfm_write_reg(0x09, 0xFF);  // PA MAXIMUM POWER
    rfm_write_reg(0x0C, 0x23);  // highest LNA gain, HF LNA BOOST

    rfm_write_reg(0x0D, 0x80);  // RegRxConfig: auto restart, no AGC, no AFC
    rfm_write_reg(0x27, 0x93);  // auto RX restart, 4 SYNC byte sent

    rfm_write_reg(0x30, 0x10);  // fixed lenght packet, CRC ON, CRC auto clear, NO ADDRESS
    rfm_write_reg(0x31, 0x40);  // packet mode, IoT disabled

    rfm_write_reg(0x35, 0x80);  //  TX start condition

    rfm_payload_size(1);
    rfm_bitwrite(0x3B, 6, 1);   // image calibration
    delay_milli(20);            // calibration delay
    rfm_mode_activeRX();
    delay_milli(100);
    ESP_LOGI(TAG, "RFM95W initialized");
}

uint8_t returns_regs(uint8_t *pointer){
    *pointer++ = rfm_read_reg(0x3e);
    *pointer = rfm_read_reg(0x3e);

    return 2;
}

/*
    selects one of 65 channels created by the library
    the first is 200kHz apart from the 902MHz lower limit
    the last is 200kHz apart from the 928MHz upper limit
    the separation between each channel is of 400kHz
*/
void rfm_channel(uint8_t channel){
    uint32_t carrier_freq;

    if(channel <= 64){
        carrier_freq = (902 * 1000 * 1000) + (200 * 1000);
        carrier_freq += channel * (400 * 1000);
        rfm_frequency_carrier(carrier_freq);
        rfm_bitwrite(0x0D, 5, 1);
        delay_micro(120);   // double the time to hop frequencies
    }
}

// Change the packet size and configure the correct time for TX to transmit
void rfm_payload_size(uint8_t packets){
    payload_size = packets; // address is included on payload size count

// t(x) = 130 us (PLL) + (3B preample + 4B sync word + 0B address + xB payload + 2B CRC)*8/ 0.05MBIT + 20 us for tolerance
// t(x) = 130 + (9 + x) * 160 + 20
// t(x) ~= 1600 + 160 * x
    tx_time = 1600 + 160 * packets;

    rfm_write_reg(0x32, payload_size & 0b00111111); // artificial limit of 63 messages
}

/* configure the address (need also to check address verification config)
void rfm_address(uint8_t address){
    rfm_write_reg(0x33, address);
}
*/

// returns the value of the RSSI
uint8_t rfm_RSSI(void){
    return rfm_read_reg(0x11) / 2;
}

// Sets the transceiver in standby mode
void rfm_mode_standby(void){
    rfm_write_reg(0x01, 0b00000001);
}

// Sets the transceiver back in RX mode
void rfm_mode_activeRX(void){
    rfm_write_reg(0x01, 0b00000101);
    delay_micro(150);   // time enough to change from TX to RX
}

// Transmits packet and returns to RX mode
bool rfm_TXtransmit(uint8_t *payload){
    bool success = false;

    rfm_mode_standby();
    rfm_TXwrite(payload);
    rfm_write_reg(0x01, 0b00000011); // radio is on TX mode

    //while(rfm_bitread(0x3F, 3) == 0);
    delay_micro(tx_time);

    if(rfm_bitread(0x3F, 3) == 1)   // check if TX_DS flag bit was set (Data Sent TX FIFO)
        success = true;             // returns true
    rfm_mode_activeRX();    // returns to RX and clears PACKETSENT flag

    return success;
}

// Returns true if received valid packet, false if not
bool rfm_RXreceive(uint8_t *payload){
    bool message_present = false;

    if(rfm_bitread(0x3F, 2) == true){   // check if RX_DS flag bit was set (Data Ready RX FIFO)
        rfm_RXread(payload);            // passes the pointer on to the RX buffer read function
        message_present = true;         // returns true
        // emptying the FIFO should clear this flag
    }

    return message_present;
}
