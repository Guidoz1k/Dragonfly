#ifndef PORTS_H_
#define PORTS_H_

void writeB(byte data);

void writeC(byte data);

void writeD(byte data);

void bitWriteB(byte bit, byte data);

void bitWriteC(byte bit, byte data);

void bitWriteD(byte bit, byte data);

byte readB();

byte readC();

byte readD();

byte bitReadB(byte bit);

byte bitReadC(byte bit);

byte bitReadD(byte bit);

void directB(byte data);

void directC(byte data);

void directD(byte data);

void bitDirectB(byte bit, byte data);

void bitDirectC(byte bit, byte data);

void bitDirectD(byte bit, byte data);

void toggleB(byte data);

void toggleC(byte data);

void toggleD(byte data);

void bitToggleB(byte bit);

void bitToggleC(byte bit);

void bitToggleD(byte bit);

#endif /* PORTS_H_ */