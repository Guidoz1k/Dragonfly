#ifndef SERIAL_H_
#define SERIAL_H_

void serialInit();

byte serialReadAvailable();

byte serialRead();

byte serialWriteAvailable();

void serialWrite(byte data);

void  serialWriteBinary(byte data);

void serialWriteString(char *data);

void serialWriteNumber(fourbyte number);

#endif /* SERIAL_H_ */