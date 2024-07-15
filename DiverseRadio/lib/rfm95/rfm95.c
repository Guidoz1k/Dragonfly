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

/* 350µs is the maximum allowed time to transmit a single byte
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

/*static*/ uint8_t rfm_read_reg(uint8_t reg){
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

/*static*/ void rfm_write_reg(uint8_t reg, uint8_t data){
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

/*static*/ bool rfm_bitread(uint8_t address, uint8_t bit){
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

/*static*/ void rfm_bitwrite(uint8_t address, uint8_t bit, bool value){
    uint8_t data = 0;

    data = rfm_read_reg(address);
    if(value == 1)
        data |= 1 << bit;
    else
        data &= ~(1 << bit);
    rfm_write_reg(address, data);
}

/*static*/ void rfm_TXwrite(uint8_t *payload){
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

/*static*/ void rfm_RXread(uint8_t *payload){
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

/*
static void rfm_bitrate(uint32_t bitrate){
    uint32_t temp_br;   // bit rate

    if( (bitrate > 0) && (bitrate <= 300000ul) ){
        temp_br = 32000000ul / bitrate;

        rfm_write_reg(0x02, (temp_br >> 8) & 0xFF);
        rfm_write_reg(0x03, temp_br & 0xFF);

        delay_micro(150);
    }

    rfm_write_reg(0x02, 0x00);  // MSB of bit rate (FXOSX = 32MHz)
    rfm_write_reg(0x03, 0x80);  // LSB of bit rate (250kHz)
}
*/

static void rfm_frequency_carrier(uint32_t freq){
    /*
    carrier frequency = step * freq_reg
    step = 15625 / 256
    freq_reg = carrier frequency / step
    */
    uint64_t temp_freq;

    if( (freq >= 902000000ul) && (freq <= 928000000ul) ){
        temp_freq = (uint64_t)((uint64_t)freq << 8) / 15625;

        rfm_write_reg(0x06, (temp_freq >> 16) & 0xFF);
        rfm_write_reg(0x07, (temp_freq >> 8) & 0xFF);
        rfm_write_reg(0x08, temp_freq & 0xFF);

        delay_micro(150);
    }
}

/*
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
*/

// ============ EXTERNAL SPI FUNCTIONS ============

// Mandatory setup initialization function
void rfm_setup(bool test){
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
    rfm_frequency_carrier(902000000);
    //rfm_bitwrite(0x24, 3, 1);           // calibrates chip after frequency change to HF

    rfm_write_reg(0x0C, 0b00100000);
    rfm_write_reg(0x0D, 0x80);  // RegRxConfig: auto restart, no AGC, no AFC


    rfm_write_reg(0x1F, 0xAA);  // preambles
    rfm_write_reg(0x26, 0x03);

    rfm_write_reg(0x27, 0b10010001);

    rfm_write_reg(0x30, 0x18);
    rfm_write_reg(0x31, 0x40);
    rfm_write_reg(0x32, 1);
    rfm_write_reg(0x35, 0x80);

    rfm_write_reg(0x09, 0xFF);

/*
    // common registers
    rfm_payload_size(1);

    // TX registers
    if(test == true)
        rfm_write_reg(0x09, 0x4F);  // minimum(standard) TX power for bench test
    else
        rfm_write_reg(0x09, 0xFF);  // maximum TX power for field test

    // RX registers
    rfm_write_reg(0x0C, 0x23);  // LNA on highest gain, with boost on
    rfm_write_reg(0x0E, 0x00);  // RSSI smoothing set to minimum
    rfm_write_reg(0x12, 0x01);  // required RxBw values for 250kHz FSK bandwidth

    // RC Oscillator register
    rfm_write_reg(0x24, 0x07);  // CLKOUT is disabled

    // Packet Handling registers
    rfm_write_reg(0x26, 0x01);  // 1B for preamble
    rfm_write_reg(0x27, 0x90);  // keeps autorestart for RX but changes syncsize to 1
    rfm_write_reg(0x30, 0x12);  // fixed lenght packet mode, no encoding, CRC on, clear FIFO when wrong CRC is received, address checking turned on
    rfm_write_reg(0x31, 0x40);  // packet mode, no IoT compatibility
    rfm_write_reg(0x35, 0x8F);  // TX starts transmission as soon as FIFO has data
//*/
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
    selects one of 64 channels created by the library
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
        delay_micro(100);   // double the time to hop frequencies
    }
}

// Change the packet size and configure the correct time for TX to transmit
void rfm_payload_size(uint8_t packets){
    payload_size = packets + 1; // address is included on payload size count

/*
    t(x) = 130 us (PLL) + (1B preample + 1B sync word + 1B address + xB payload + 2B CRC)*8/ 0.25MBIT + 20 us for tolerance
    t(x) = 130 + (5 + x) * 32 + 20
    t(x) = 340 + 5 * x
*/
    tx_time = 340 + 5 * packets; // Coincidently it is the same for the nRF24L01+

    rfm_write_reg(0x32, payload_size & 0b00111111); // artificial limit of 63 messages
}

/* configure the address
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

    gpio_set_level(PIN_TEST, 0);
    rfm_mode_standby();
    rfm_TXwrite(payload);
    gpio_set_level(PIN_TEST, 1);
    rfm_write_reg(0x01, 0b00000011); // radio is on TX mode
    gpio_set_level(PIN_TEST, 0);

    while(rfm_bitread(0x3F, 3) == 0);
    
    gpio_set_level(PIN_TEST, 1);
    if(rfm_bitread(0x3F, 3) == 1)   // check if TX_DS flag bit was set (Data Sent TX FIFO)
        success = true;             // returns true
    rfm_mode_activeRX();    // returns to RX and clears PACKETSENT flag

    gpio_set_level(PIN_TEST, 0);

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
