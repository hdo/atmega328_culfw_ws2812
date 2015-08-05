/* 
 * Copyright by R.Koenig
 * Inspired by code from Dirk Tostmann
 * License: GPL v2
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/parity.h>
#include <string.h>

#include "board.h"
#include "delay.h"
#include "rf_send.h"
#include "rf_receive.h"
#include "led.h"
#include "cc1100.h"
#include "display.h"
#include "fncollection.h"


// For FS20 we time the complete message, for KS300 the rise-fall distance
// FS20  NULL: 400us high, 400us low
// FS20  ONE:  600us high, 600us low
// KS300 NULL  854us high, 366us low
// KS300 ONE:  366us high, 854us low

#define FS20_ZERO      400     //   400uS
#define FS20_ONE       600     //   600uS
#define FS20_PAUSE      10     // 10000mS
#define EM_ONE         800     //   800uS
#define EM_ZERO        400     //   400uS

uint16_t credit_10ms;


#define MAX_SNDMSG 6    // FS20: 4 or 5 + CRC, FHT: 5+CRC
#define MAX_SNDRAW 7    // MAX_SNDMSG*9/8 (parity bit)

static void
send_bit(uint8_t bit)
{
  CC1100_OUT_PORT |= _BV(CC1100_OUT_PIN);         // High
  my_delay_us(bit ? FS20_ONE : FS20_ZERO);

  CC1100_OUT_PORT &= ~_BV(CC1100_OUT_PIN);       // Low
  my_delay_us(bit ? FS20_ONE : FS20_ZERO);
}


static void sendraw(uint8_t *msg, uint8_t sync, uint8_t nbyte, uint8_t bitoff, 
                uint8_t repeat, uint8_t pause);

// msg is with parity/checksum already added
static void
sendraw(uint8_t *msg, uint8_t sync, uint8_t nbyte, uint8_t bitoff,
                uint8_t repeat, uint8_t pause)
{
  // 12*800+1200+nbyte*(8*1000)+(bits*1000)+800+10000 
  // message len is < (nbyte+2)*repeat in 10ms units.
  int8_t i, j, sum = (nbyte+2)*repeat;
  if (credit_10ms < sum) {
    DS_P(PSTR("LOVF\r\n"));
    return;
  }
  credit_10ms -= sum;

  LED_ON();


  if(!cc_on)
    set_ccon();
  ccTX();                                       // Enable TX 
  do {
    for(i = 0; i < sync; i++)                   // sync
      send_bit(0);
    if(sync)
      send_bit(1);
    
    for(j = 0; j < nbyte; j++) {                // whole bytes
      for(i = 7; i >= 0; i--)
        send_bit(msg[j] & _BV(i));
    }
    for(i = 7; i > bitoff; i--)                 // broken bytes
      send_bit(msg[j] & _BV(i));

    my_delay_ms(pause);                         // pause

  } while(--repeat > 0);

  if(tx_report) {                               // Enable RX
    ccRX();
  } else {
    ccStrobe(CC1100_SIDLE);
  }


  LED_OFF();
}

static int
abit(uint8_t b, uint8_t *obuf, uint8_t *obyp, uint8_t obi)
{
  uint8_t oby = * obyp;
  if(b)
    obuf[oby] |= _BV(obi);
  if(obi-- == 0) {
    oby++;
    if(oby < MAX_SNDRAW)
      *obyp = oby;
    obi = 7; obuf[oby] = 0;
  }
  return obi;
}

void
addParityAndSendData(uint8_t *hb, uint8_t hblen,
                uint8_t startcs, uint8_t repeat)
{
  uint8_t iby, obuf[MAX_SNDRAW], oby;
  int8_t ibi, obi;

  hb[hblen] = cksum1(startcs, hb, hblen);
  hblen++;

  // Copy the message and add parity-bits
  iby=oby=0;
  ibi=obi=7;
  obuf[oby] = 0;

  while(iby<hblen) {
    obi = abit(hb[iby] & _BV(ibi), obuf, &oby, obi);
    if(ibi-- == 0) {
      obi = abit(parity_even_bit(hb[iby]), obuf, &oby, obi);
      ibi = 7; iby++;
    }
  }
  if(obi-- == 0) {                   // Trailing 0 bit: no need for a check
    oby++; obi = 7;
  }

  sendraw(obuf, 12, oby, obi, repeat, FS20_PAUSE);
}

void
addParityAndSend(char *in, uint8_t startcs, uint8_t repeat)
{
  uint8_t hb[MAX_SNDMSG], hblen;
  hblen = fromhex(in+1, hb, MAX_SNDMSG-1);
  addParityAndSendData(hb, hblen, startcs, repeat);
}


void
fs20send(char *in)
{
  addParityAndSend(in, 6, 3);
}
