#ifndef SERIAL_H_
#define SERIAL_H_

void serialInit9600(void);

uint8_t serialReadAvailable(void);

uint8_t serialRead(void);

uint8_t serialWriteAvailable(void);

void serialWrite(uint8_t data);

void  serialWriteBinary(uint8_t data);

void serialWriteString(char *data);

void serialWriteNumber(uint32_t number);

#endif /* SERIAL_H_ */