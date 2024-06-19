#include "atc.h"

char in_buffer[PAYLOAD]; // command char + payload

uint8_t read_command(uint16_t *payload_out){
    uint8_t command_ready = 0;
    uint8_t i = 0;

    if(serial_read_size() != 0){    // detects something on buffer
        delay_milli(100);           // give it time to load the entire message

        if(serial_read_size() == (PAYLOAD)){        // compare if message is on standard size
            serial_read_chars(in_buffer, PAYLOAD);  // load message to input buffer
            for(i = 0; i < sizeof(command); i++){   // detect the command char
                if(in_buffer[0] == command[i])
                    command_ready = i;
            }
            if(command_ready != 0){     // command_ready stores which match it got from the first char
                switch(command_ready){  // validates the payload and only then loads it to the output pointer
                case 1: // 'S'
                    break;
                case 2: // 'D'
                    break;
                case 3: // 'H'
                    break;
                default: // INVALID or no match
                    break;
                }
            }
        }

        else{
            serial_flush();     // if message is not on standard size, discard it
        }

    }

    return command_ready;
}

