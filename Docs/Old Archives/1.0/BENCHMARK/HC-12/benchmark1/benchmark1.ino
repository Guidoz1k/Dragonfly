#include <SoftwareSerial.h>

SoftwareSerial hc12(11,12);
unsigned char uto[5]={127,127,127,127,0};
unsigned char op[3]={0,0,0};
unsigned long int tempo=0;
unsigned long int tempo2=0;
unsigned long int cont=0;

void setup(){
  Serial.begin(9600);
  hc12.begin(9600);
  while(!Serial);
  Serial.println("Serial ready");
  tempo2=micros();
}

void loop(){
  tempo=0;

  if( (millis()-tempo2) > 1000 ){
    hc12.write(uto,5);
    tempo=micros();

    while(hc12.available()!=3);
    tempo=micros()-tempo;
  
    hc12.readBytes(op,3);
    tempo2=millis();
    Serial.print(" (");
    Serial.print(cont);
    Serial.print("): ");
    Serial.println(tempo);
    cont++;
    }

}
