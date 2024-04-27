/* isso
 * nao
 * ta
 * pronto
 */
#ifndef nRF_h
#define nRF_h

#include <Arduino.h>
#include <SPI.h>

#define CE_pin    7
#define CSN_pin   8
#define MOSI_pin  11
#define MISO_pin  12
#define SCK_pin   13

#define INBYTES   5
#define OUTBYTES  5
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
byte data_receive[INBYTES];
byte data_transmit[OUTBYTES];

/*after init:
 * 00 - 0x7F
 * 01 - 0x3F
 * 02 - 0x01
 * 03 - 0x03
 * 04 - 0x00
 * 05 - 0x78
 * 06 - 0x26
 * 07 - 0x0E
 * 17 - 0x11
*/

class nRF{

  public:
    void nRF_init()
    void nRF_write(byte address,byte bytes,byte printa)
    byte nRF_read(byte address,byte printa)
    void nRF_send(byte info,byte printa)
    void nRF_bitwrite(byte address,byte bite,byte val)
    boolean nRF_bitread(byte address,byte bite)
    void nRF_transmit()
    boolean nRF_RXcheck()
    void nRF_receive()
    boolean RF_status_bitread(byte bite)
    void RF_status_bitwrite(byte bite,byte val)
    void g_print(byte info)
    void nRF_scanregs(byte start,byte finish,boolean printa)
  private:
    
}


#endif
