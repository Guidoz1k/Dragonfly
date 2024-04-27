#include <SPI.h>

#define MOSI_pin  12
#define MISO_pin  11
#define SCK_pin   13
#define CSN_pin   10
#define CE_pin    9
#define IRQ_pin   8

#define INBYTES 1
#define OUTBYTES 1

int address = 0;
int spi_status = 0;
int data_transmit[OUTBYTES];
int data_receive[INBYTES];

byte variavel = 0;

void setup(){
  nRF_init();
  Serial.begin(9600);
  Serial.println("COMM CHECK");
}

void loop(){
  variavel++;
  Serial.println(variavel);
  nRF_transmit(variavel);
  Serial.flush();
  delay(100);
}

//////////////////////////////////////////////////// "nRF.h"

void nRF_init(){
  delay(100);                   // transceiver startup delay
  pinMode(CE_pin, OUTPUT);
  pinMode(CSN_pin, OUTPUT);
  pinMode(MOSI_pin, OUTPUT);
  pinMode(MISO_pin, INPUT);
  pinMode(SCK_pin, OUTPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
  digitalWrite(CE_pin, HIGH);
  digitalWrite(CSN_pin, HIGH);

  nRF_write(0x11, INBYTES);    // incoming bytes in RX_PW_P0

  nRF_write(0x00, B01111111);
  nRF_write(0x01, B00000000);
  nRF_write(0x02, B00000001);
  nRF_write(0x04, B00000000);
  nRF_write(0x05, B01111000);
  nRF_write(0x06, B00100110);

  nRF_bitwrite(0x07, 4, 1);
  nRF_bitwrite(0x07, 5, 1);
  nRF_bitwrite(0x07, 6, 1);

  nRF_send(B11100001);  // flush TX
  nRF_send(B11100010);  // flush RX
}

void nRF_write(byte address, byte bytes){
  byte data[2] = {0, 0};

  digitalWrite(CSN_pin, LOW);
  data[0] = SPI.transfer(address + 32); // write byte format
  data[1] = SPI.transfer(bytes);
  digitalWrite(CSN_pin, HIGH);

  spi_status = data[0];
}

byte nRF_read(byte address){
/*
 * printa = 0 for no serial print
 * printa = 1 for serial printing the result
 * printa = 2 for serial printing the status too
 */
  byte data[2] = {0, 0};

  digitalWrite(CSN_pin, LOW);
  data[0] = SPI.transfer(address + 0);  // read byte format
  data[1] = SPI.transfer(B00000000);    // read result
  digitalWrite(CSN_pin, HIGH);

  spi_status = data[0];

  return data[1];
}

void nRF_send(byte info){
  byte data = 0;

  digitalWrite(CSN_pin, LOW);
  data = SPI.transfer(info);    // write byte format
  digitalWrite(CSN_pin, HIGH);

  spi_status = data;
}

void nRF_bitwrite(byte address, byte bite, byte val){
  byte data;

  data = nRF_read(address);
  bitWrite(data, bite, val);
  nRF_write(address, data);
}

boolean nRF_bitread(byte address, byte bite){
  boolean state;
  byte data;

  data = nRF_read(address);
  if(bitRead(data, bite))
    state = true;
  else
    state = false;

  return state;
}

void nRF_transmit(byte data){
  byte check = 0;

  nRF_send(B11100001);      // flush TX
  nRF_bitwrite(0x07, 4, 1); // clear MAX_RT flag

  digitalWrite(CSN_pin, LOW);
  check = SPI.transfer(B10100000); // load TX payload
  SPI.transfer(data);
  digitalWrite(CSN_pin, HIGH);

  digitalWrite(CE_pin, LOW);
  delayMicroseconds(1000);
  nRF_bitwrite(0x00, 0, 0);  // TX mode
  delayMicroseconds(1000);
  digitalWrite(CE_pin, HIGH);
  delayMicroseconds(1000);
  nRF_bitwrite(0x00, 0, 1);  // RX mode

  nRF_send(B11100001);      // flush TX
  nRF_bitwrite(0x07, 4, 1); // clear MAX_RT flag
}

boolean nRF_RXcheck(){
  boolean received;

  if(nRF_bitread(0x17, 0))
    received = false;
  else
    received = true;

  return received;
}

void nRF_receive(){
  byte data[INBYTES + 1];

  digitalWrite(CSN_pin, LOW);
  data[0] = SPI.transfer(B01100001); // read RX payload
  for(int i = 1; i <= INBYTES; i++)
    data[i] = SPI.transfer(B00000000);
  digitalWrite(CSN_pin, HIGH);

  nRF_send(B11100001);  // flush RX
  for(int i; i < INBYTES; i++)
    data_receive[i] = data[i + 1];
}

void g_print(byte info){
  for(int i = 7; i >= 0; i--){
    Serial.print(bitRead(info, i));
    }
}

void nRF_scanregs(byte start, byte finish, boolean printa){
  for(int i = start; i <= finish; i++){
    Serial.print("Reg: ");
    if(i<0x10)
      Serial.print("0");
    Serial.print(i, HEX);
    Serial.print(" = ");
    if(printa == false){
      g_print(nRF_read(i));
      Serial.println("");
      }
    else
      Serial.println(nRF_read(i), HEX);
    }
}
