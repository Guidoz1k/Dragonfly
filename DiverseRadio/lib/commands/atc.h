#ifndef __ATC_H
#define __ATC_H

#include <stdio.h>

#include "serial.h"
#include "delay.h"

#define COMMANDS    4   // number of commands
#define PAYLOAD     6   // 1 char for command + ammount of chars used in the payload

char command[] = {
    0,      // INVALID COMMAND
    'S',    // SYSTEM COMMAND
    'D',    // DIVERSITY COMMAND
    'H',    // HOPPING COMMAND
};

uint8_t read_command(uint16_t *payload_out);

#endif /* __ATC_H */