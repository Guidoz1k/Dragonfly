#ifndef __ATC_H
#define __ATC_H

#include <stdbool.h>
#include <stdint.h>

char command[] = {
    0,      // INVALID COMMAND
    'S',    // SYSTEM COMMAND
    'D',    // DIVERSITY COMMAND
    'H',    // HOPPING COMMAND
};

uint8_t read_command(uint16_t *payload_out);

#endif /* __ATC_H */