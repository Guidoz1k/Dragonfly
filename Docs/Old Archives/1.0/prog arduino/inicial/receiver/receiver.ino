#include <SoftwareSerial.h>

#define PITCH     10  //0
#define ROLL      11  //1
#define YAW       9   //2
#define THROTTLE  3   //3
#define BATSENSOR 4   //sensor de bateria

#define SERVOMIN    75
#define SERVOMAX    238
#define SERVOLIM    50
#define INPUTS      4
#define THROTTLEDIV 31
#define THROTTLEMIN 147
#define THROTTLEMAX 210

#define CICLE       100

int serv[INPUTS];                 //posicao efetiva do servo
unsigned char pin[INPUTS];        //pinos a serem escritos
unsigned char uto[INPUTS];        //posicoes recebidas do controle
unsigned char pos[INPUTS];        //pitch, roll, yaw, throttle pra sincronia de pacotes
unsigned char recebido=0;         //status de recebimento de sinal
unsigned char rec[3]={110,0,212}; //bytes de retorno
unsigned char contagem=0;         //contagem de ciclos para resposta
/* código sms
   * 0 - nada recebido
   * 1 - recebido sem warning
   * 2 - recebido com warning de bateria
   * 3 - recebido com warning de sinal
   * 4 - recebido warning de baterial e sinal */

SoftwareSerial hc12(7,8);

void setup(){
  Serial.begin(9600);
  hc12.begin(2400);
  pinMode(PITCH,OUTPUT);
  pinMode(ROLL,OUTPUT);
  pinMode(YAW,OUTPUT);
  pinMode(THROTTLE,OUTPUT);

  pinMode(BATSENSOR,INPUT);

  pin[0]=PITCH;
  pin[1]=ROLL;
  pin[2]=YAW;
  pin[3]=THROTTLE;


  //sync();
  pos[0]=0;
  pos[1]=1;
  pos[2]=2;
  pos[3]=3;
  warmup();
}

void loop(){
  recebido=0;

  if(hc12.available()){
    recebido=hc12.readBytes(uto,4);
    recebe(recebido);
    }
  if(recebido==INPUTS)
    for(int i=0;i<INPUTS;i++)
      control(i,-1);

  contagem++;
}

void control(int i,int value){  //value = -1 para usar uto
  int mapa=0,dif=0;

  if(value==-1)
    value=uto[pos[i]];

  serv[i]=value;
  if(i<3)
    mapa=map(value,0,255,SERVOMIN,SERVOMAX);
  else
    if(i==3)
      mapa=map(value,0,255,THROTTLEMIN,THROTTLEMAX);

  analogWrite(pin[i],mapa);
}

void warmup(){
  for(int i=0;i<INPUTS;i++){
    uto[i]=255/2;
    control(i,-1);
    delay(500);
    digitalWrite(pin[i],LOW);
    delay(500);
    }
  uto[3]=0;
  control(3,-1);
}

void recebe(unsigned char recebido){
  unsigned char errosinal=0;
  unsigned char warningbat=0;
  unsigned char aenviar=0;

  if( rec[0]!=110 || rec[2]!=212 || recebido<4)
    errosinal=1;
  if(digitalRead(BATSENSOR==0)==HIGH)
    warningbat=1;

  if(errosinal)
    if(warningbat)
      aenviar=4;
    else
      aenviar=3;
  else
    if(warningbat)
      aenviar=2;
    else
      aenviar=1;

  rec[1]=aenviar;
  if(contagem>=CICLE){
    hc12.write(rec,3);
    contagem=0;
  }

}

