#include <SPI.h>
#include <Wire.h>

//////////////////////////////////////////////////// SPI nRF pins

#define CE_pin    9
#define CSN_pin   10
#define MOSI_pin  11
#define MISO_pin  12
#define SCK_pin   13

//////////////////////////////////////////////////// custom pins

#define BAT_ALARM 2
#define LIGHTS1   3
#define LIGHTS2   4
#define TER_LED   5

// PCA values
#define ELEVATOR  0
#define AILERON   1
#define RUDDER    2
#define THROTTLE  3
#define FLAPS     4

//////////////////////////////////////////////////// values

#define SERVOMIN  70    // minimum byte value of servo motor
#define SERVOMAX  240   // maximum byte value of servo motor

#define PCA_ADDR  0x40  // pwm controller address
#define PCA_PWM0  0x08  // PCA LED0_OFF_L register
#define PWM_MIN   115   // ~0.56ms in a 4096 scale of 20ms for servos
#define PWM_MAX   393   // ~1.92ms in a 4096 scale of 20ms for servos
#define ESC_MIN   240   // ~1.20ms in a 4096 scale of 20ms for ESC
#define ESC_MAX   390   // ~1.90ms in a 4096 scale of 20ms for ESC

#define OUTBYTES  1     // bytes transmitted
#define INBYTES   5     // bytes received

/* data transmit structure
 * 0 - drone alarm flag
*/
byte data_transmit[OUTBYTES];
/* data_receive structure
 * 0 - pitch
 * 1 - roll
 * 2 - yaw
 * 3 - throttle
 * 4 - switches:
 *    7   - lights
 *    6-0 - flaps (0-127)
*/
byte data_receive[INBYTES];
byte spi_status=0;

void setup(){
  //PCA_init();
  nRF_init();

  Serial.begin(9600);
  while(!Serial);
  Serial.println(" Slave module: Serial ready");

  if(nRF_read(0x00)!=0x7F){
    Serial.println("nRF24l01+ not working");
    digitalWrite(TER_LED,1);
    //while(1);
    }
  /*nRF_scanregs(0x00,0x06,1);
  nRF_scanregs(0x07,0x07,1);
  nRF_scanregs(0x17,0x17,0);
  Serial.println("");*/
}

void loop(){
  while(!nRF_RXcheck());
  delayMicroseconds(100);
  Serial.println("loop");

  nRF_receive();
  //output_package();
  //read_alarm();
  nRF_transmit();
}

//////////////////////////////////////////////////// custom

void output_package(){
  output_axis();
  outptut_toggles(data_receive[FLAPS]);
}

void output_axis(){
  for(int i=0;i<=RUDDER;i++)
    PCA_servo(i,data_receive[i]);
  PCA_esc(data_receive[3]);
}

void read_alarm(){
  data_transmit[0]=!digitalRead(BAT_ALARM);
}

void outptut_toggles(byte value){
  digitalWrite(LIGHTS1,bitRead(value,7));
  digitalWrite(LIGHTS2,bitRead(value,6));

  value=(value&B00111111)<<2;
  PCA_servo(FLAPS,value);
}

void PCA_init(){
  byte value=0;

  Wire.begin();
  Wire.setClock(400000);

  PCA_write(0x00,B00010001);       //sleep
  PCA_write(B11111110,B01111110);
    /* prescaler = 126
     * the prescaler is set to get 50hz but
     * the clock is unknown (~26MHz). if
     * correct clock (25MHz), ps should be 121)
     */
  PCA_write(0x00,B00000001);
  delayMicroseconds(600);
  PCA_write(0x00,B10100001);
}

byte PCA_esc(byte value){
  int value2;

  value2=map(value,0,255,ESC_MIN,ESC_MAX);
  return PCA_pwm(3,value2);
}

byte PCA_servo(byte pin,byte value){
  int value2;

  value2=map(value,0,255,PWM_MIN,PWM_MAX);
  return PCA_pwm(pin,value2);
}

byte PCA_pwm(byte pin,int off){
  Wire.beginTransmission(PCA_ADDR);
  Wire.write(PCA_PWM0+4*pin);
  Wire.write(off);
  Wire.write(off>>8);
  return Wire.endTransmission();
}

byte PCA_write(byte control,byte data){
  Wire.beginTransmission(PCA_ADDR);
  Wire.write(control);
  Wire.write(data);
  return Wire.endTransmission();
}

byte PCA_read(byte control){
  byte error;

  Wire.beginTransmission(PCA_ADDR);
  Wire.write(control);
  error=Wire.endTransmission();

  Wire.requestFrom(PCA_ADDR,1);
  return Wire.read();
}

//////////////////////////////////////////////////// "nRF.h"

void nRF_init(){
  delay(100);                   // transceiver startup delay
  pinMode(CE_pin,OUTPUT);
  pinMode(CSN_pin,OUTPUT);
  pinMode(MOSI_pin,OUTPUT);
  pinMode(MISO_pin,INPUT);
  pinMode(SCK_pin,OUTPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
  digitalWrite(CE_pin,HIGH);
  digitalWrite(CSN_pin,HIGH);

  nRF_write(0x11,INBYTES);    // incoming bytes in RX_PW_P0

  nRF_write(0x00,B01111111);
  nRF_write(0x01,B00000000);
  nRF_write(0x02,B00000001);
  nRF_write(0x04,B00000000);
  nRF_write(0x05,B01111000);
  nRF_write(0x06,B00100110);

  nRF_bitwrite(0x07,4,1);
  nRF_bitwrite(0x07,5,1);
  nRF_bitwrite(0x07,6,1);

  nRF_send(B11100001);  // flush TX
  nRF_send(B11100010);  // flush RX
}




void nRF_write(byte address,byte bytes){
  byte data[2]={0,0};

  digitalWrite(CSN_pin,LOW);
  data[0]=SPI.transfer(address+32); // write byte format
  data[1]=SPI.transfer(bytes);
  digitalWrite(CSN_pin, HIGH);

  spi_status=data[0];
}

byte nRF_read(byte address){
/*
 * printa = 0 for no serial print
 * printa = 1 for serial printing the result
 * printa = 2 for serial printing the status too
 */
  byte data[2]={0,0};

  digitalWrite(CSN_pin,LOW);
  data[0]=SPI.transfer(address+0); // read byte format
  data[1]=SPI.transfer(B00000000); // read result
  digitalWrite(CSN_pin, HIGH);

  spi_status=data[0];

  return data[1];
}

void nRF_send(byte info){
  byte data=0;

  digitalWrite(CSN_pin,LOW);
  data=SPI.transfer(info); // write byte format
  digitalWrite(CSN_pin, HIGH);

  spi_status=data;
}

void nRF_bitwrite(byte address,byte bite,byte val){
  byte data;

  data=nRF_read(address);
  bitWrite(data,bite,val);
  nRF_write(address,data);
}

boolean nRF_bitread(byte address,byte bite){
  boolean state;
  byte data;

  data=nRF_read(address);
  if(bitRead(data,bite))
    state=true;
  else
    state=false;

  return state;
}

void nRF_transmit(){
  byte data[OUTBYTES+1];

  nRF_send(B11100001);  // flush TX
  nRF_bitwrite(0x07,4,1); // clear MAX_RT flag

  digitalWrite(CSN_pin,LOW);
  data[0]=SPI.transfer(B10100000); // load TX payload
  for(int i=1;i<=OUTBYTES;i++)
    data[i]=SPI.transfer(data_transmit[i-1]);
  digitalWrite(CSN_pin, HIGH);

  digitalWrite(CE_pin, LOW);
  delayMicroseconds(1000);
  nRF_bitwrite(0x00,0,0);  // TX mode
  delayMicroseconds(1000);
  digitalWrite(CE_pin, HIGH);
  delayMicroseconds(1000);
  nRF_bitwrite(0x00,0,1);  // RX mode

  nRF_send(B11100001);  // flush TX
  nRF_bitwrite(0x07,4,1); // clear MAX_RT flag
}

boolean nRF_RXcheck(){
  boolean received;

  if(nRF_bitread(0x17,0))
    received=false;
  else
    received=true;

  return received;
}

void nRF_receive(){
  byte data[INBYTES+1];

  digitalWrite(CSN_pin,LOW);
  data[0]=SPI.transfer(B01100001); // read RX payload
  for(int i=1;i<=INBYTES;i++)
    data[i]=SPI.transfer(B00000000);
  digitalWrite(CSN_pin, HIGH);

  nRF_send(B11100001);  // flush RX
  for(int i;i<INBYTES;i++)
    data_receive[i]=data[i+1];
}

void g_print(byte info){
  for(int i=7;i>=0;i--){
    Serial.print(bitRead(info,i));
    }
}

void nRF_scanregs(byte start,byte finish,boolean printa){
  for(int i=start;i<=finish;i++){
    Serial.print("Reg: ");
    if(i<0x10)
      Serial.print("0");
    Serial.print(i,HEX);
    Serial.print(" = ");
    if(printa==false){
      g_print(nRF_read(i));
      Serial.println("");
      }
    else
      Serial.println(nRF_read(i),HEX);
    }
}
