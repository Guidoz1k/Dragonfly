#ifndef PORTS_H_
#define PORTS_H_

void writeB(uint8_t data);

void writeC(uint8_t data);

void writeD(uint8_t data);

void bitWriteB(uint8_t bit, uint8_t data);

void bitWriteC(uint8_t bit, uint8_t data);

void bitWriteD(uint8_t bit, uint8_t data);

uint8_t readB(void);

uint8_t readC(void);

uint8_t readD(void);

uint8_t bitReadB(uint8_t bit);

uint8_t bitReadC(uint8_t bit);

uint8_t bitReadD(uint8_t bit);

void directB(uint8_t data);

void directC(uint8_t data);

void directD(uint8_t data);

void bitDirectB(uint8_t bit, uint8_t data);

void bitDirectC(uint8_t bit, uint8_t data);

void bitDirectD(uint8_t bit, uint8_t data);

void toggleB(uint8_t data);

void toggleC(uint8_t data);

void toggleD(uint8_t data);

void bitToggleB(uint8_t bit);

void bitToggleC(uint8_t bit);

void bitToggleD(uint8_t bit);

#endif /* PORTS_H_ */