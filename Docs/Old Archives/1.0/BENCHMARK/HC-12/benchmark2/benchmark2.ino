#include <SoftwareSerial.h>

SoftwareSerial hc12(11,12);
unsigned char uto[3]={110,0,212};
unsigned char op[5]={0,0,0,0,0};

void setup(){
  Serial.begin(9600);
  hc12.begin(9600);
}

void loop(){
  while(hc12.available()!=5);

  hc12.readBytes(op,5);
  hc12.write(uto,3);
}
