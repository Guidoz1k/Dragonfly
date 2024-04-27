#include <SPI.h>
#include "NRF24L01.h"

#define CE       9
#define CSN      10
#define SCK      13
#define MOSI     11
#define MISO     12
#define IRQ      8

unsigned char TX_ADDRESS[5]  = {0x34,0x43,0x10,0x10,0x01};

unsigned char rx_buf[32];

void setup() 
{
  pinMode(CE,  OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(CSN, OUTPUT);
  pinMode(MOSI,  OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(IRQ, INPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
  SPI_RW_Reg(FLUSH_RX,0); 
  Serial.begin(9600);
  digitalWrite(IRQ, 1);
  digitalWrite(CSN, 1);

  digitalWrite(CE, 0);
  SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, 5);
  SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, 5);

  SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);
  SPI_RW_Reg(WRITE_REG + RF_CH, 0x3F);
  SPI_RW_Reg(WRITE_REG + RX_PW_P0, 32);
  SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x2F);
  SPI_RW_Reg(WRITE_REG + CONFIG, 0x7f);
  digitalWrite(CE, 1);
}

void loop() 
{
  for(;;)
  {
    unsigned char status = SPI_Read(STATUS);                         // read register STATUS's value
    if(status&RX_DR)                                                 // if receive data ready (TX_DS) interrupt
    {
      SPI_Read_Buf(RD_RX_PLOAD, rx_buf, 32);             // read playload to rx_buf
      SPI_RW_Reg(FLUSH_RX,0);                                        // clear RX_FIFO
      for(int i=0; i<32; i++)
      {
          Serial.print(" ");
          Serial.print(rx_buf[i]);                              // print rx_buf
      }
      Serial.println(status);
    }
    SPI_RW_Reg(WRITE_REG+STATUS,status);                             // clear RX_DR or TX_DS or MAX_RT interrupt flag
    delay(250);
  }
}

unsigned char SPI_RW_Reg(unsigned char reg, unsigned char value)
{
  unsigned char status;

  digitalWrite(CSN, 0);
  status = SPI.transfer(reg);
  SPI.transfer(value);
  digitalWrite(CSN, 1);

  return(status);
}

unsigned char SPI_Read(unsigned char reg)
{
  unsigned char reg_val;

  digitalWrite(CSN, 0);
  SPI.transfer(reg);
  reg_val = SPI.transfer(0);
  digitalWrite(CSN, 1);

  return(reg_val);
}

unsigned char SPI_Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char bytes)
{
  unsigned char status,i;

  digitalWrite(CSN, 0);
  status = SPI.transfer(reg);

  for(i=0;i<bytes;i++)
  {
    pBuf[i] = SPI.transfer(0);
  }

  digitalWrite(CSN, 1);

  return(status);
}

unsigned char SPI_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char bytes)
{
  unsigned char status,i;

  digitalWrite(CSN, 0);
  status = SPI.transfer(reg);
  for(i=0;i<bytes; i++)
  {
    SPI.transfer(*pBuf++);
  }
  digitalWrite(CSN, 1);
  return(status);
}
