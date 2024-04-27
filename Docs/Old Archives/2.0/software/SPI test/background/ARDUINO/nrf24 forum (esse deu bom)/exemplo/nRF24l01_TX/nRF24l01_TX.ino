// compiler shit

#include <SPI.h>
#define READ_REG        0x00  // Define read command to register
#define WRITE_REG       0x20  // Define write command to register
#define RD_RX_PLOAD     0x61  // Define RX payload register address
#define WR_TX_PLOAD     0xA0  // Define TX payload register address
#define FLUSH_TX        0xE1  // Define flush TX register command
#define FLUSH_RX        0xE2  // Define flush RX register command
#define REUSE_TX_PL     0xE3  // Define reuse TX payload register command
#define NOP             0xFF  // Define No Operation, might be used to read status register
//***************************************************
#define RX_DR    0x40
#define TX_DS    0x20
#define MAX_RT   0x10
//***************************************************
// SPI(nRF24L01) registers(addresses)
#define CONFIG          0x00  // 'Config' register address
#define EN_AA           0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR       0x02  // 'Enabled RX addresses' register address
#define SETUP_AW        0x03  // 'Setup address width' register address
#define SETUP_RETR      0x04  // 'Setup Auto. Retrans' register address
#define RF_CH           0x05  // 'RF channel' register address
#define RF_SETUP        0x06  // 'RF setup' register address
#define STATUS          0x07  // 'Status' register address
#define OBSERVE_TX      0x08  // 'Observe TX' register address
#define CD              0x09  // 'Carrier Detect' register address
#define RX_ADDR_P0      0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1      0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2      0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3      0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4      0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5      0x0F  // 'RX address pipe5' register address
#define TX_ADDR         0x10  // 'TX address' register address
#define RX_PW_P0        0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1        0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2        0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3        0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4        0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5        0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS     0x17  // 'FIFO Status Register' register address

#define CE       9
#define CSN      10
#define SCK      13
#define MOSI     11
#define MISO     12
#define IRQ      8

unsigned char TX_ADDRESS[5]  = {0x34,0x43,0x10,0x10,0x01};

unsigned char tx_buf[21] = {0};

void setup() 
{
  pinMode(CE,  OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(CSN, OUTPUT);
  pinMode(MOSI,  OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(IRQ, INPUT);
  Serial.begin(9600);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(IRQ, 0);
  digitalWrite(CSN, 1);

  digitalWrite(CE, 0);
  SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, 5);    // Writes TX_Address to nRF24L01
  SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, 5); // RX_Addr0 same as TX_Adr for Auto.Ack

  SPI_RW_Reg(WRITE_REG + EN_AA, 0);
  SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0);
  SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);
  SPI_RW_Reg(WRITE_REG + RF_CH, 0x3F);
  SPI_RW_Reg(WRITE_REG + RX_PW_P0, 32);
  SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x2F);
  SPI_RW_Reg(WRITE_REG + CONFIG, 0x7e);
  SPI_Write_Buf(WR_TX_PLOAD,tx_buf,32);
  digitalWrite(CE, 1);

SPI_RW_Reg(FLUSH_TX,0);

}

void loop() 
{
  int k = 0;
  for(;;)
  {
    for(int i=0; i<32; i++)
        tx_buf[i] = k++;        
    unsigned char status = SPI_Read(STATUS);                   // read register STATUS's value
    if(status&TX_DS)                                           // if receive data ready (TX_DS) interrupt
    {
      SPI_RW_Reg(FLUSH_TX,0);                                  
      SPI_Write_Buf(WR_TX_PLOAD,tx_buf,32);       // write playload to TX_FIFO
    }
    //SPI_RW_Reg(WRITE_REG+STATUS,status);                     // clear RX_DR or TX_DS or MAX_RT interrupt flag
    delay(1000);

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

unsigned char SPI_Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char uint8_ts)
{
  unsigned char status,i;

  digitalWrite(CSN, 0);
  status = SPI.transfer(reg);

  for(i=0;i<uint8_ts;i++)
  {
    pBuf[i] = SPI.transfer(0);
  }

  digitalWrite(CSN, 1);

  return(status);
}

unsigned char SPI_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char uint8_ts)
{
  unsigned char status,i;

  digitalWrite(CSN, 0);
  status = SPI.transfer(reg);
  for(i=0;i<uint8_ts; i++)
  {
    SPI.transfer(*pBuf++);
  }
  digitalWrite(CSN, 1);
  return(status);
}
