/*
 * Copyright by D.Tostmann
 * Inspired by code from TI.com AN
 * License: GPL v2
 */

#include "board.h"
#ifdef HAS_MBUS
#include <string.h>
#include <avr/pgmspace.h>
#include "cc1100.h"
#include "delay.h"
#include "rf_receive.h"
#include "display.h"

#include "rf_mbus.h"
#include "mbus/mbus_defs.h"
#include "mbus/smode_rf_settings.h"
#include "mbus/tmode_rf_settings.h"
#include "mbus/mbus_packet.h"
#include "mbus/manchester.h"
#include "mbus/3outof6.h"

// RX - Buffers
uint8 RXpacket[291];
uint8 RXbytes[584];

static uint8_t mbus_mode = WMBUS_NONE;
RXinfoDescr RXinfo;

static void halRfReadFifo(uint8* data, uint8 length) {
  CC1100_ASSERT;

  cc1100_sendbyte( CC1100_RXFIFO|CC1100_READ_BURST );
  for (uint8_t i = 0; i < length; i++)
    data[i] = cc1100_sendbyte( 0 );

  CC1100_DEASSERT;
}

static void halRfWriteReg( uint8_t reg, uint8_t value ) {
  cc1100_writeReg( reg, value );
}

static uint8_t rf_mbus_on(uint8_t force) {

  // already in RX?
  if (!force && (cc1100_readReg( CC1100_MARCSTATE ) == MARCSTATE_RX))
    return 0;

  // init RX here, each time we're idle
  RXinfo.state = 0;

  ccStrobe( CC1100_SIDLE );
  while((cc1100_readReg( CC1100_MARCSTATE ) != MARCSTATE_IDLE));
  ccStrobe( CC1100_SFTX  );
  ccStrobe( CC1100_SFRX  );

  // Initialize RX info variable
  RXinfo.lengthField = 0;           // Length Field in the wireless MBUS packet
  RXinfo.length      = 0;           // Total length of bytes to receive packet
  RXinfo.bytesLeft   = 0;           // Bytes left to to be read from the RX FIFO
  RXinfo.pByteIndex  = RXbytes;     // Pointer to current position in the byte array
  RXinfo.format      = INFINITE;    // Infinite or fixed packet mode
  RXinfo.start       = TRUE;        // Sync or End of Packet
  RXinfo.complete    = FALSE;       // Packet Received
  RXinfo.mode        = mbus_mode;   // Wireless MBUS mode

  // Set RX FIFO threshold to 4 bytes
  halRfWriteReg(CC1100_FIFOTHR, RX_FIFO_START_THRESHOLD);
  // Set infinite length 
  halRfWriteReg(CC1100_PKTCTRL0, INFINITE_PACKET_LENGTH);

  ccStrobe( CC1100_SRX   );
  while((cc1100_readReg( CC1100_MARCSTATE ) != MARCSTATE_RX));

  RXinfo.state = 1;

  return 1; // this will indicate we just have re-started RX
}

static void rf_mbus_init(uint8_t mode) {

  CLEAR_BIT( GDO0_DDR, GDO0_BIT );
  CLEAR_BIT( GDO2_DDR, GDO2_BIT );

  mbus_mode = WMBUS_NONE;

  EIMSK &= ~_BV(CC1100_INT);                 // disable INT - we'll poll...
  SET_BIT( CC1100_CS_DDR, CC1100_CS_PIN );   // CS as output

  CC1100_DEASSERT;                           // Toggle chip select signal
  my_delay_us(30);
  CC1100_ASSERT;
  my_delay_us(30);
  CC1100_DEASSERT;
  my_delay_us(45);

  ccStrobe( CC1100_SRES );                   // Send SRES command
  my_delay_us(100);

  // load configuration
  switch (mode) {
    case WMBUS_SMODE:
      for (uint8_t i = 0; i<200; i += 2) {
        if (sCFG(i)>0x40)
          break;
        cc1100_writeReg( sCFG(i), sCFG(i+1) );
      }
      break;
    case WMBUS_TMODE:
      for (uint8_t i = 0; i<200; i += 2) {
        if (tCFG(i)>0x40)
          break;
        cc1100_writeReg( tCFG(i), tCFG(i+1) );
      }
      break;
    default:
      return;
  }

  mbus_mode = mode;

  ccStrobe( CC1100_SCAL );

  memset( &RXinfo, 0, sizeof( RXinfo ));

  my_delay_ms(4);
}

void rf_mbus_task(void) {
  uint8 bytesDecoded[2];
  uint8 fixedLength;

  if (mbus_mode == WMBUS_NONE)
    return;

  switch (RXinfo.state) {
    case 0:
      rf_mbus_on( TRUE );
      return;

     // RX active, awaiting SYNC
    case 1:
      if (bit_is_set(GDO2_PIN,GDO2_BIT)) {
        RXinfo.state = 2;
      }
      break;

    // awaiting pkt len to read
    case 2:
      if (bit_is_set(GDO0_PIN,GDO0_BIT)) {
        // Read the 3 first bytes
        halRfReadFifo(RXinfo.pByteIndex, 3);

        // - Calculate the total number of bytes to receive -
        if (RXinfo.mode == WMBUS_SMODE) {
          // S-Mode
          // Possible improvment: Check the return value from the deocding function,
          // and abort RX if coding error. 
          if (manchDecode(RXinfo.pByteIndex, bytesDecoded) != MAN_DECODING_OK) {
            RXinfo.state = 0;
            return;
	  }		
          RXinfo.lengthField = bytesDecoded[0];
          RXinfo.length = byteSize(1, 0, (packetSize(RXinfo.lengthField)));
        } else {
          // T-Mode
          // Possible improvment: Check the return value from the deocding function,
          // and abort RX if coding error. 
          if (decode3outof6(RXinfo.pByteIndex, bytesDecoded, 0) != DECODING_3OUTOF6_OK) {
            RXinfo.state = 0;
            return;
	  }		
          RXinfo.lengthField = bytesDecoded[0];
          RXinfo.length = byteSize(0, 0, (packetSize(RXinfo.lengthField)));
        }

	// check if incoming data will fit into buffer
	if (RXinfo.length>sizeof(RXbytes)) {
          RXinfo.state = 0;
          return;
 	}

        // we got the length: now start setup chip to receive this much data
        // - Length mode -
        // Set fixed packet length mode is less than 256 bytes
        if (RXinfo.length < (MAX_FIXED_LENGTH)) {
          halRfWriteReg(CC1100_PKTLEN, (uint8)(RXinfo.length));
          halRfWriteReg(CC1100_PKTCTRL0, FIXED_PACKET_LENGTH);
          RXinfo.format = FIXED;
        }
      
        // Infinite packet length mode is more than 255 bytes
        // Calculate the PKTLEN value
        else {
          fixedLength = RXinfo.length  % (MAX_FIXED_LENGTH);
          halRfWriteReg(CC1100_PKTLEN, (uint8)(fixedLength)); 
        }
      
        RXinfo.pByteIndex += 3;
        RXinfo.bytesLeft   = RXinfo.length - 3;
      
        // Set RX FIFO threshold to 32 bytes
        RXinfo.start = FALSE;
        RXinfo.state = 3;
        halRfWriteReg(CC1100_FIFOTHR, RX_FIFO_THRESHOLD);
      }
      break;

    // awaiting more data to be read
    case 3:
      if (bit_is_set(GDO0_PIN,GDO0_BIT)) {
        // - Length mode -
        // Set fixed packet length mode is less than MAX_FIXED_LENGTH bytes
        if (((RXinfo.bytesLeft) < (MAX_FIXED_LENGTH )) && (RXinfo.format == INFINITE)) {
          halRfWriteReg(CC1100_PKTCTRL0, FIXED_PACKET_LENGTH);
          RXinfo.format = FIXED;
        }
  
        // Read out the RX FIFO
        // Do not empty the FIFO (See the CC110x or 2500 Errata Note)
        halRfReadFifo(RXinfo.pByteIndex, RX_AVAILABLE_FIFO - 1);

        RXinfo.bytesLeft  -= (RX_AVAILABLE_FIFO - 1);
        RXinfo.pByteIndex += (RX_AVAILABLE_FIFO - 1);

      }
      break;
  }

  // END OF PAKET
  if (!bit_is_set(GDO2_PIN,GDO2_BIT) && RXinfo.state>1) {
    halRfReadFifo(RXinfo.pByteIndex, (uint8)RXinfo.bytesLeft);
    RXinfo.complete = TRUE;

    // decode!
    uint16_t rxStatus = PACKET_CODING_ERROR;

    if (RXinfo.mode == WMBUS_SMODE)
      rxStatus = decodeRXBytesSmode(RXbytes, RXpacket, packetSize(RXinfo.lengthField));
    else
      rxStatus = decodeRXBytesTmode(RXbytes, RXpacket, packetSize(RXinfo.lengthField));

    if (rxStatus == PACKET_OK) {

      DC( 'b' );

      for (uint8_t i=0; i < packetSize(RXpacket[0]); i++) {
        DH2( RXpacket[i] );
//	DC( ' ' );
      }

      DNL();
    }
    RXinfo.state = 0;
    return;
  }

  rf_mbus_on( FALSE );
}

static void mbus_status(void) {
  switch (mbus_mode) {
    case WMBUS_SMODE:
      DS_P(PSTR("SMODE"));
      break;
    case WMBUS_TMODE:
      DS_P(PSTR("TMODE"));
      break;
    default:
      DS_P(PSTR("OFF"));
  }
  DNL();
}


void rf_mbus_func(char *in) {
  if((in[1] == 'r') && in[2]) {     // Reception on
    if(in[2] == 's') {
      rf_mbus_init(WMBUS_SMODE);
    } else if(in[2] == 't') {
      rf_mbus_init(WMBUS_TMODE);
    } else {                        // Off
      rf_mbus_init(WMBUS_NONE);
    }	

  } else if(in[1] == 's') {         // Send
    return;

  }

  mbus_status();
}

#endif
