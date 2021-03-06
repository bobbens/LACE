

#include "timer.h"

#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "uart.h"
#include "event.h"


typedef volatile struct timer_s {
   uint16_t left;
   void (*func)(int);
} timer_t;


static timer_t timers[ MAX_TIMERS ];


/**
 * @brief Overflow vector.
 */
ISR( TIMER0_COMPA_vect )
{
   int i;
   event_t evt;

   /* Reset watchdog. */
   wdt_reset();

   for (i=0; i<MAX_TIMERS; i++) {
      /* Only interested in active timers. */
      if (timers[i].left == 0)
         continue;

      /* Decrement timer. */
      timers[i].left--;
      if (timers[i].left > 0)
         continue;
  
      /* Run callback if exists. */
      if (timers[i].func != NULL)
         timers[i].func( i );
   
      /* Push event. */
      evt.type          = EVENT_TYPE_TIMER;
      evt.timer.timer   = i;
      event_push( &evt );
   }
}


void timer_init (void)
{
   int i;

   /* Enable timer. */
   TIFR0  = _BV(OCF0A) | _BV(OCF0B) | _BV(TOV0); /* Clear interrupt flags. */
   TIMSK0 = 0;
   PRR   &= ~_BV(PRTIM0);

   /* Clear timers. */
   for (i=0; i<MAX_TIMERS; i++)
      timers[i].left = 0;

   /* Set up timer. */
   /* CTC Mode
    *
    *               f_clk
    * f_ctc = -----------------
    *         2 * N * (1 + TOP)
    *
    *    f_clk
    * ------------- - 1 = TOP
    * f_ctc * 2 * N
    *
    *  f_clk   = 20e6
    *  f_ctc   = 1e3
    *  N       = 256
    *  TOP     = 38.0625 ~= 38
    *
    *  Since we generate 2 interrupts each cycle, we'll need it to be at twice.
    *
    *  TOP*2 = 76
    */
   TCCR0A = _BV(WGM01); /* CTC mode. */
   TCCR0B = _BV(CS02); /* 256 prescaler. */
   TCNT0  = 0; /* Clear timer. */
   OCR0A  = 76;
   OCR0B  = 0;
   TIMSK0 = _BV(OCIE0A); /* Enable interrupt. */


   /* Set up watchdog timer. */
   wdt_reset(); /* Just in case. */
   wdt_enable( WDTO_250MS );
}


void timer_exit (void)
{
   PRR |= _BV(PRTIM0); /* Disable timer. */

   /* Disable watchdog timer. */
   wdt_reset(); /* Important according to datasheet. */
   wdt_disable();
}


void timer_start( int timer, uint16_t ms, void (*func)(int) )
{
   timers[ timer ].left = ms;
   timers[ timer ].func = func;
}


void timer_stop( int timer )
{
   timers[ timer ].left = 0;
}


