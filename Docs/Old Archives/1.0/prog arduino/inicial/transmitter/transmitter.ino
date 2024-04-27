#include <SoftwareSerial.h>

#define PITCH     A0 //0
#define ROLL      A1 //1
#define YAW       A2 //2
#define THROTTLE  A3 //3
#define BUTTON1   12 //botao throttle
#define BUTTON2   13 //botao manche
#define VELOX     10 //velocimetro
#define LEDBATD   4  //led indicador de bateria fraca no drone
#define LEDBATC   5  //led indicador de bateria fraca no controle
#define LEDSIGNAL 6  //led indicador de perda de pacote
#define LEDLOST   7  //led indicador de perda de sinal
#define COMMCHECK 8  //led indicador de comunicacao ok
#define BATSENSOR 9  //entrada do sensor de bateria do controle

#define SPIKE       15  //tolerancia do sinal de entrada
#define INPUTS      4   //entradas a serem enviadas
#define SERVOMIN    70  //
#define SERVOMAX    240 // parametros do servo
#define SERVOLIM    40  //
#define THROTTLEDIV 60  // sampling to throttle
#define SILENCETIME 500 //millis para trigga perda de sinal

unsigned char pin[INPUTS];
unsigned char uto[INPUTS];
int uto2[INPUTS];
int turbo=0;                // estado do throttle em memoria
unsigned long tempomudo=0;  // contagem de tempo sem receber sinal
unsigned long tempoforce=0; // contagem de tempo do throttle

SoftwareSerial hc12(2,3);

void setup(){
  delay(750);           //tempo de teste do alarma bat
  Serial.begin(9600);
  hc12.begin(2400);

  pinMode(PITCH,INPUT);
  pinMode(ROLL,INPUT);
  pinMode(YAW,INPUT);
  pinMode(THROTTLE,INPUT);
  pinMode(BUTTON1,INPUT);
  pinMode(BUTTON2,INPUT);

  pinMode(VELOX,OUTPUT);
  pinMode(LEDBATD,OUTPUT);
  pinMode(LEDBATC,OUTPUT);
  pinMode(LEDSIGNAL,OUTPUT);
  pinMode(LEDLOST,OUTPUT);
  pinMode(COMMCHECK,OUTPUT);

  pinMode(BATSENSOR,INPUT);

  pin[0]=PITCH;
  pin[1]=ROLL;
  pin[2]=YAW;
  pin[3]=THROTTLE;

  uto[0]=127;
  uto[1]=127;
  uto[2]=127;
  uto[3]=0;
  turbo=0;

  velocimetro(-1);
  timingresposta(0);

  hc12.write(uto,4);
}

void loop(){
  reading();
  if(receive()!=0)
    hc12.write(uto,4);
  velocimetro(uto[3]);
  checkBat();
}

void reading(){
  int mapa=0,dif=0;

  for(int i=0;i<INPUTS;i++){
    mapa=analogRead(pin[i]);
    if(abs(mapa-uto2[i])>SPIKE){
      uto2[i]=mapa;
      dif=uto2[i]-uto[i];
      if(abs(dif)>SERVOLIM)
        if(dif>0)
          uto[i]+=SERVOLIM;
        else
          uto[i]-=SERVOLIM;
      else
        uto[i]=uto2[i];
      }
    uto[i]=uto2[i]/4;
    }

  if(digitalRead(BUTTON1)==LOW){
    turbo+=(uto[3]-127)/THROTTLEDIV;
    if(turbo>255)
      turbo=255;
    if(turbo<0)
      turbo=0;
    uto[3]=turbo;
    }
  turbo=uto[3];
}

unsigned char receive(){
  /*
   * atualiza sms
   * 
   * envia 0 para não enviar nada
   * envia 1 para enviar de novo
   */
  unsigned char recebido[3]={0,0,0};
  unsigned char pacotes=0;
  unsigned char enviar=0; //0 nao envia, 1 envia
  unsigned char sms=0;
  /* código sms
   * 0 - nada recebido
   * 1 - recebido sem warning
   * 2 - recebido com warning de bateria
   * 3 - recebido com warning de sinal
   * 4 - recebido warning de baterial e sinal */
int aux=0;

  aux=hc12.available();

  if(aux){
    pacotes=hc12.readBytes(recebido,3);
    if( pacotes<3 || recebido[0]!=110 || recebido[2]!=212 )
      sms=3;
    switch(recebido[1]){
      case 1:
        if(sms==0)
          sms=1;
        break;
      case 2:
        if(sms==3)
          sms=4;
        else
          sms=2;
        break;
      case 3:
        sms=3;
        break;
      case 4:
        sms=4;
      }
    enviar=1;
    timingresposta(0);
    }
  else{
    aux=timingresposta(1);
    if(aux!=0){
      sms=0;
      if(aux==2)
        enviar=1;
      else
        enviar=0;
      }
    }

  if(aux!=0)
    atualizaLeds(sms);

  return enviar;
}

unsigned char timingresposta(unsigned char recebido){
/*
 * recebe zero pra iniciar a contagem
 * recebe um pra testar a contagem
 * 
 * envia zero pra contagem em progresso
 * envia um pra estouro na contagem
 * envia dois pra broadcast emergencial
 */
  int dif=millis()-tempomudo;
  unsigned char retorna=0;

  if(recebido==0)
    tempomudo=millis();
  else
    if( dif>SILENCETIME && dif<(SILENCETIME*2) )
      retorna=1;
    if(dif>(SILENCETIME*2)){  // emite um sinal a cada SILENCETIME
      tempomudo=millis()-SILENCETIME;
      retorna=2;
    }

  return retorna;
}

void atualizaLeds(unsigned char recebido){
  /* código sms
   * 0 - nada recebido
   * 1 - recebido sem warning
   * 2 - recebido com warning de bateria
   * 3 - recebido com warning de sinal
   * 4 - recebido warning de baterial e sinal */

  switch(recebido){
    case 0: // nada recebido
      digitalWrite(LEDBATD,LOW);
      digitalWrite(LEDSIGNAL,LOW);
      digitalWrite(LEDLOST,HIGH);
      digitalWrite(COMMCHECK,LOW);
      break;
    case 1: // recebido sem warning
      digitalWrite(LEDBATD,LOW);
      digitalWrite(LEDSIGNAL,LOW);
      digitalWrite(LEDLOST,LOW);
      digitalWrite(COMMCHECK,HIGH);
      break;
    case 2: // recebido com warning de bateria
      digitalWrite(LEDBATD,HIGH);
      digitalWrite(LEDSIGNAL,LOW);
      digitalWrite(LEDLOST,LOW);
      digitalWrite(COMMCHECK,HIGH);
      break;
    case 3: // recebido com warning de sinal
      digitalWrite(LEDBATD,LOW);
      digitalWrite(LEDSIGNAL,HIGH);
      digitalWrite(LEDLOST,LOW);
      digitalWrite(COMMCHECK,HIGH);
      break;
    case 4: // recebido warning de baterial e sinal
      digitalWrite(LEDBATD,HIGH);
      digitalWrite(LEDSIGNAL,HIGH);
      digitalWrite(LEDLOST,LOW);
      digitalWrite(COMMCHECK,HIGH);
      break;
    }
}

void velocimetro(unsigned char cond){
// -1 pra warmup, positivo pra atualiza

  int mapa=0;

  if(cond==-1)
    mapa=map(0,0,255,SERVOMIN,SERVOMAX);
  else
    mapa=map(cond,0,255,SERVOMIN,SERVOMAX);

  analogWrite(VELOX,mapa);
}

void checkBat(){
  if(digitalRead(BATSENSOR)==HIGH)
    digitalWrite(LEDBATC,HIGH);
  else
    digitalWrite(LEDBATC,LOW);
}

