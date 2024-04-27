#include <Wire.h>

#define PCA_ADDR 0x40   // pwm controller address
#define PCA_PWM0 0x08   // PCA LED0_OFF_L register
#define PWM_MIN  115    // ~0.56ms in a 4096 scale of 20ms
#define PWM_MAX  393    // ~1.92ms in a 4096 scale of 20ms

byte PCA_ADDRESS=0;

void setup(){
  Serial.begin(9600);
  while(!Serial);
  PCA_init(PCA_ADDR);
}

void loop(){
  for(int i=0;i<256;i++){
    PCA_servo(1,i);
    PCA_servo(0,i);
    delay(10);
    }
  delay(500);
  for(int i=255;i>0;i--){
    PCA_servo(1,i);
    PCA_servo(0,i);
    delay(10);
    }
 delay(500);
}
////////////////////////////////////////////////////////

void PCA_init(byte address){
  byte value=0;

  PCA_ADDRESS=address;
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

byte PCA_servo(byte pin,byte value){
  int value2;
  value2=map(value,0,255,PWM_MIN,PWM_MAX);
  return PCA_pwm(pin,value2);
}

byte PCA_pwm(byte pin,int off){
  Wire.beginTransmission(PCA_ADDRESS);
  Wire.write(PCA_PWM0+4*pin);
  Wire.write(off);
  Wire.write(off>>8);
  return Wire.endTransmission();
}

byte PCA_write(byte control,byte data){
  Wire.beginTransmission(PCA_ADDRESS);
  Wire.write(control);
  Wire.write(data);
  return Wire.endTransmission();
}

byte PCA_read(byte control){
  byte error;

  Wire.beginTransmission(PCA_ADDRESS);
  Wire.write(control);
  error=Wire.endTransmission();

  Wire.requestFrom(PCA_ADDRESS,1);
  return Wire.read();
}

