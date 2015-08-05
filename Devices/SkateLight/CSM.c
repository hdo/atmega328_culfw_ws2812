/* Copyright Rudolf Koenig, 2008.
   Released under the GPL Licence, Version 2
   Inpired by the MyUSB USBtoSerial demo, Copyright (C) Dean Camera, 2008.
*/

#include <avr/boot.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include <string.h>

#include "board.h"

#include "spi.h"
#include "cc1100.h"
#include "clock.h"
#include "delay.h"
#include "display.h"
#include "serial.h"
#include "fncollection.h"
#include "led.h"
#include "ringbuffer.h"
#include "rf_receive.h"
#include "rf_send.h"
#include "ttydata.h"
#include "ws2812.h"




const PROGMEM t_fntab fntab[] = {

  { 'B', prepare_boot },
  { 'C', ccreg },
  { 'F', fs20send },
  { 'R', read_eeprom },
  { 'V', version },
  { 'W', write_eeprom },
  { 'X', set_txreport },

  { 'e', eeprom_factory_reset },
  { 'l', ledfunc },
  { 't', gettime },
  { 'x', ccsetpa },

  { 0, 0 },
};

volatile uint32_t ticks;

int
main(void)
{
  wdt_disable();


  led_init();
  LED_ON();

  spi_init();

//  eeprom_factory_reset("xx");
  eeprom_init();

//  led_mode = 2;

  // if we had been restarted by watchdog check the REQ BootLoader byte in the
  // EEPROM ...
//  if(bit_is_set(MCUSR,WDRF) && eeprom_read_byte(EE_REQBL)) {
//    eeprom_write_byte( EE_REQBL, 0 ); // clear flag
//    start_bootloader();
//  }

  // Setup the timers. Are needed for watchdog-reset
  OCR0A  = 249;                            // Timer0: 0.008s = 8MHz/256/250 == 125Hz
  TCCR0B = _BV(CS02);
  TCCR0A = _BV(WGM01);
  TIMSK0 = _BV(OCIE0A);

  TCCR1A = 0;
  TCCR1B = _BV(CS11) | _BV(WGM12);         // Timer1: 1us = 8MHz/8

  clock_prescale_set(clock_div_1);		   // see <avr/power.h>

  MCUSR &= ~(1 << WDRF);                   // Enable the watchdog
  wdt_enable(WDTO_2S);

  uart_init( UART_BAUD_SELECT_DOUBLE_SPEED(UART_BAUD_RATE,F_CPU) );


  tx_init();
  input_handle_func = analyze_ttydata;

  display_channel = DISPLAY_USB;



  LED_OFF();

  sei();

  // enable FS20 receive
  set_txreport("X21\n");

  ws2812_init(); // init led colors

  for(;;) {
    uart_task();
    RfAnalyze_Task();
    Minute_Task();
    ws2812_task(ticks);
  }

}
