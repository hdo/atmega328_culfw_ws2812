#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "board.h"
#include "led.h"
#ifdef XLED
#include "xled.h"
#endif
#include "fncollection.h"
#include "clock.h"
#include "display.h"
#include "battery.h"
#include "joy.h"
#include "fht.h"
#include "fswrapper.h"                 // fs_sync();
#include "rf_send.h"                   // credit_10ms
#include "mysleep.h"
#include "pcf8833.h"
#ifdef HAS_USB
#include "cdc.h"
#endif
#include "rf_router.h"                  // rf_router_flush();
#include "ntp.h"
#ifdef HAS_ONEWIRE
#include "onewire.h"
#endif
#ifdef HAS_VZ
#include "vz.h"
#endif

#if defined (HAS_IRRX) || defined (HAS_IRTX)
#include "ir.h"
uint8_t ir_ticks = 0;
uint8_t ir_ticks_thrd = 0;
#endif

volatile uint32_t ticks;
volatile uint8_t  clock_hsec;

// count & compute in the interrupt, else long runnning tasks would block
// a "minute" task too long
ISR(TIMER0_COMPA_vect, ISR_BLOCK)
{

#ifdef HAS_IRTX     //IS IRTX defined ?
         if (! ir_send_data() ) {   //If IR-Sending is in progress, don't receive
         #ifdef HAS_IRRX  //IF also IRRX is define
          ir_sample(); // call IR sample handler
       #endif
     }
#elif defined (HAS_IRRX)
     ir_sample(); // call IR sample handler
#endif

#if defined (HAS_IRTX) || defined (HAS_IRRX)
     // if IRRX is compiled in, timer runs 125x faster ... 
     if (++ir_ticks<125) 
       return;
       
     ir_ticks = 0;
#endif

     // 125Hz
     ticks++; 
     if(++clock_hsec>=125)  
	  clock_hsec = 0;

#ifdef HAS_NTP
  ntp_hsec++;
  if(ntp_hsec >= 125) {
    ntp_sec++;
    ntp_hsec = 0;
  }
#endif

#ifdef HAS_FHT_8v
  if(fht8v_timeout)
    fht8v_timeout--;
#endif
#ifdef HAS_FHT_80b
  if(fht80b_timeout != FHT_TIMER_DISABLED)
    fht80b_timeout--;
#endif


}

void
gettime(char *unused)
{
  uint8_t *p = (uint8_t *)&ticks;
  DH2(p[3]);
  DH2(p[2]);
  DH2(p[1]);
  DH2(p[0]);
  DNL();
}

#ifdef HAS_ETHERNET
//Return time
clock_time_t
clock_time()
{
  return (clock_time_t)ticks;
}
#endif

void
Minute_Task(void)
{
  static uint8_t last_tick;
  if((uint8_t)ticks == last_tick)
    return;
  last_tick = (uint8_t)ticks;
  wdt_reset();

  // 125Hz
#ifdef XLED
  if ((ticks % 12) == 0) {
    if ( xled_pattern & _BV(xled_pos++) ) {
      LED_ON();
    } else {
      LED_OFF();
    }
  }
  xled_pos &= 15;
#endif
#ifdef HAS_FHT_8v
  if(fht8v_timeout == 0)
    fht8v_timer();
#endif
#ifdef HAS_FHT_80b
  if(fht80b_timeout == 0)
    fht80b_timer();
#endif
#ifdef HAS_RF_ROUTER
  if(rf_router_sendtime && --rf_router_sendtime == 0)
    rf_router_flush();
#endif

// OneWire hSec-Task 
// Check if a running conversion is done
// if HMS Emulation is on, and the Minute timer has expired
#ifdef HAS_ONEWIRE
    onewire_HsecTask ();
#endif

  if(clock_hsec>0)     // Note: this can skip some hsecs
    return;

#ifndef XLED
  // 1Hz
  if(led_mode & 2)
    LED_TOGGLE();
#endif

  // one second, 1% duty cycle, 10ms resolution => this is simple ;-)
  if (credit_10ms < MAX_CREDIT)
    credit_10ms += 1;

// if HMS Emulation is on, check the HMS timer
#ifdef HAS_ONEWIRE
    onewire_SecTask ();
#endif
#ifdef HAS_VZ
    vz_sectask();
#endif

#if defined(HAS_SLEEP) && defined(JOY_PIN1)
  if(joy_inactive < 255)
    joy_inactive++;

  if(sleep_time && joy_inactive == sleep_time) {
    if(USB_IsConnected)
      lcd_switch(0);
    else
      dosleep();
  }
#endif

#ifdef HAS_NTP
  if((ntp_sec & NTP_INTERVAL_MASK) == 0)
    ntp_sendpacket();
#endif

  static uint8_t clock_sec;
  clock_sec++;
  if(clock_sec != 60)                   // minute from now on
    return;
  clock_sec = 0;

#ifdef HAS_FHT_80b
  fht80b_minute++;
  if(fht80b_minute >= 60)
    fht80b_minute = 0;
#endif

#if defined(HAS_LCD) && defined(BAT_PIN)
  bat_drawstate();
#endif
}
