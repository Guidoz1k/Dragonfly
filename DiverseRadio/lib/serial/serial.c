#include "serial.h"

uint8_t read_timeout = pdMS_TO_TICKS(10);

void serial_setup(){
    uart_config_t configs = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_set_pin(UART_NUM_0, 1, 3, -1, -1);
    uart_param_config(UART_NUM_0, &configs);
    uart_driver_install(UART_NUM_0, 2048, 0, 0, NULL, 0);
}

uint8_t serial_write_word(uint32_t number, uint8_t size, bool newline){
    uint8_t buffer[10] = {MAXSIZE};
    uint8_t chars = 10;
    uint8_t i;

    // convert the number to individual 
    for(i = 0; i < 10; i++){
        buffer[9 - i] = number % 10 + 48;
        number /= 10;
    }

    // find how many characters are needed for given number, change '0' for ' ', accounting for a number == 0
    i = 0;
    while((buffer[i] == '0') && (i < 9)){
        buffer[i] = ' ';
        i++;
        chars--;
    }

    if(size >= chars){
        // fit number in size requested
        for(i = 0; i < size; i++)
            buffer[i] = buffer[i + (10 - size)];
        uart_write_bytes(UART_NUM_0, buffer, size);
    }
    else
        serial_write_string("-=ERROR!=-", 0);

    // finalizes the process
    if(newline == true)
        serial_new_line();
    return chars;
}

void serial_write_byte(uint8_t number, base_t base, bool newline){
    uint8_t i;
    uint8_t size = 0;
    char buffer[9];

    switch(base){
    case BIN:
        for(i = 0; i < 8; i++){
            buffer[7 - i] = number % 2 + 48;
            number /= 2;
        }
        for(i = 9; i >= 5; i--)
            buffer[i] = buffer[i - 1];
        buffer[4] = '.';

        size = 9;
        break;
    case DEC:
        buffer[0] = ((number / 100) % 10) + 48;
        buffer[1] = ((number / 10) % 10) + 48;
        buffer[2] = (number % 10) + 48;

        size = 3;
        break;
    case HEX:
        buffer[0] = '0';
        buffer[1] = 'x';
        buffer[2] = ((number >> 4) & 0x0F) + 48;
        if(buffer[2] > (9 + 48))
            buffer[2] += 7;
        buffer[3] = (number & 0x0F) + 48;
        if(buffer[3] > (9 + 48))
            buffer[3] += 7;

        size = 4;
        break;
    default:
        break;
    }

    if(size != 0){
        uart_write_bytes(UART_NUM_0, buffer, size);
        if(newline == true)
            serial_new_line();
    }
}

void serial_new_line(void){
    char buffer = '\n';

    uart_write_bytes(UART_NUM_0, &buffer, 1);
}

void serial_write_string(const char *pointer, bool newline){
	uint8_t counter = 0;
    char buffer[MAXSIZE + 1] = {0};

	while((counter < MAXSIZE) && (*pointer != '\0')){
        buffer[counter++] = *(pointer++);
	}
    if(newline == true)
        buffer[counter] = '\n';
    else
        counter--;

    uart_write_bytes(UART_NUM_0, buffer, counter + 1);
}

uint8_t serial_read_chars(uint8_t *buffer, uint8_t size){
    
    uint8_t chars_read = 0;

    if(size <= MAXSIZE)
        chars_read = uart_read_bytes(UART_NUM_0, buffer, size, read_timeout);

    return chars_read;
}

char serial_read_singlechar(void){
    char uto = 0;

    uart_read_bytes(UART_NUM_0, &uto, 1, read_timeout);

    return uto;
}

uint8_t serial_read_size(void){
    size_t size_IDF = 0;
    uint8_t size = 0;

    uart_get_buffered_data_len(UART_NUM_0, &size_IDF);
    size = (uint8_t)size_IDF;

    return size;
}

void serial_flush(void){
    uart_flush(UART_NUM_0);
}
