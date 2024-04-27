#include <SPI.h>

#define CE_pin    7
#define CSN_pin   8
#define MOSI_pin  11
#define MISO_pin  12
#define SCK_pin   13

#define OUTBYTES  5
#define INBYTES   3
#define TIMEOUT   50000 // micros
#define COUNTTIME 1000  // millis

byte RF_status=0;
/* RF_status individual bits:
 * 0 = CRC
 * 1 = transceiver RPD below -64db flag
 * 2 = connection timeout
 * 3 = controller battery alarm
 * 4 = drone battery alarm
 */
byte uto[5]={127,128,129,130,0};
byte op[3]={0,0,0};
unsigned long int silence_time=0; // no answer time
unsigned long int count_time=0;   // time to count packages

int cont=0;

void setup(){
  Serial.begin(9600);
  while(!Serial);
  Serial.println(" Master module: Serial ready");

  nRF_init();
  if(!nRF_read(0x00,0)){
    Serial.println("nRF24l01+ not working");
    }
  nRF_scanregs(0x00,0x06,0);
  nRF_scanregs(0x07,0x07,0);
  nRF_scanregs(0x17,0x17,0);
  Serial.println("");

  count_timer(0);
}

void loop(){
  while(!count_timer(1));
  count_timer(0);

  nRF_transmit();
  silence_timer(0);
  while(!nRF_RXcheck()&&!silence_timer(1));

  Serial.print(cont);
  Serial.print(" - ");
  nRF_receive();
  if(!silence_timer(1)){
    Serial.println(micros()-silence_time);
    }

  silence_timer(0);
  cont++;
}

void nRF_init(){
  delay(100); // transceiver startup delay
  pinMode(CE_pin,OUTPUT);
  pinMode(CSN_pin,OUTPUT);
  pinMode(MOSI_pin,OUTPUT);
  pinMode(MISO_pin,INPUT);
  pinMode(SCK_pin,OUTPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
  digitalWrite(CE_pin,HIGH);
  digitalWrite(CSN_pin,HIGH);

  nRF_write(0x11,INBYTES,0);    // incoming bytes in RX_PW_P0

  nRF_write(0x00,B01111111,0);
  nRF_write(0x01,B00000000,0);
  nRF_write(0x02,B00000001,0);
  nRF_write(0x04,B00000000,0);
  nRF_write(0x05,B01111000,0);
  nRF_write(0x06,B00100110,0);

  nRF_bitwrite(0x07,4,1);
  nRF_bitwrite(0x07,5,1);
  nRF_bitwrite(0x07,6,1);

  nRF_send(B11100001,0);  // flush TX
  nRF_send(B11100010,0);  // flush RX

  nRF_write(0x01,0x3F,0);  //
}

void nRF_write(byte address,byte bytes,byte printa){
  byte data[2]={0,0};

  digitalWrite(CSN_pin,LOW);
  data[0]=SPI.transfer(address+32); // write byte format
  data[1]=SPI.transfer(bytes);
  digitalWrite(CSN_pin, HIGH);

  if(printa==1){
    Serial.print("\n Reading status: ");
    g_print(data[0]);
    }

}

byte nRF_read(byte address,byte printa){
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

  if(printa!=0){
    Serial.print("\n Reading data: ");
    g_print(data[1]);
    }

  if(printa==2){
    Serial.print("\n Reading status: ");
    g_print(data[0]);
    }

  return data[1];
}

void nRF_send(byte info,byte printa){
  byte data=0;

  digitalWrite(CSN_pin,LOW);
  data=SPI.transfer(info); // write byte format
  digitalWrite(CSN_pin, HIGH);

  if(printa==1){
    Serial.println("\n Reading data[0]");
    g_print(data);
    }
}

void nRF_bitwrite(byte address,byte bite,byte val){
  byte data;

  data=nRF_read(address,0);
  bitWrite(data,bite,val);
  nRF_write(address,data,0);
}

boolean nRF_bitread(byte address,byte bite){
  boolean state;
  byte data;

  data=nRF_read(address,0);
  if(bitRead(data,bite))
    state=true;
  else
    state=false;

  return state;
}

void nRF_transmit(){
  byte data[OUTBYTES+1];

  nRF_send(B11100001,0);  // flush TX
  nRF_bitwrite(0x07,4,1); //clear MAX_RT flag

  digitalWrite(CSN_pin,LOW);
  data[0]=SPI.transfer(B10100000); // load TX payload
  for(int i=1;i<=OUTBYTES;i++)
    data[i]=SPI.transfer(uto[i-1]);
  digitalWrite(CSN_pin, HIGH);

  digitalWrite(CE_pin, LOW);
  delayMicroseconds(1000);
  nRF_bitwrite(0x00,0,0); // TX mode
  delayMicroseconds(1000);
  digitalWrite(CE_pin, HIGH);
  delayMicroseconds(1000);
  nRF_bitwrite(0x00,0,1); // RX mode

  nRF_send(B11100001,0);  // flush TX
  nRF_bitwrite(0x07,4,1); //clear MAX_RT flag
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

  nRF_send(B11100001,0);  // flush RX
  for(int i;i<INBYTES;i++)
    op[i]=data[i+1];

  if(nRF_bitread(0x09,0))
    RF_status_bitwrite(1,0);
  else
    RF_status_bitwrite(1,1);
}

boolean RF_status_bitread(byte bite){
  boolean state;

  if(bitRead(RF_status,bite))
    state=true;
  else
    state=false;

  return state;
}

void RF_status_bitwrite(byte bite,byte val){
  bitWrite(RF_status,bite,val);
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
      g_print(nRF_read(i,0));
      Serial.println("");
      }
    else
      Serial.println(nRF_read(i,0),HEX);
    }
}

boolean silence_timer(boolean setting){
/*
 * setting:
 *  1 - measure delay
 *  0 - resets delay
 */
  boolean return_value=true;  

  if(setting)
    if( (micros()-silence_time) >= TIMEOUT )
      return_value=true;
    else
      return_value=false;
  else
    silence_time=micros();

  return return_value;
}

boolean count_timer(boolean setting){
/*
 * setting:
 *  1 - measure delay
 *  0 - resets delay
 */
  boolean return_value=true;  

  if(setting)
    if( (millis()-count_time) >= COUNTTIME )
      return_value=true;
    else
      return_value=false;
  else
    count_time=millis();

  return return_value;
}

