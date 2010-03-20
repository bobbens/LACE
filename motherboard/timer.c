

#include "timer.h"

#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "event.h"


typedef volatile struct timer_s {
   uint16_t left;
   void (*func)(int);
} timer_t;


static timer_t timers[ MAX_TIMERS ];


/**
 * @brief Overflow vector.
 */
ISR( TIMER0_OVF_vect )
{
   int i;
   event_t evt;

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

   /* Clear timers. */
   for (i=0; i<MAX_TIMERS; i++)
      timers[i].left = 0;

   /* Set up timer. */
   TCCR0A = _BV(WGM01); /* CTC mode. */
   TCCR0B = _BV(CS01) | _BV(CS00); /* 64 prescaler. */
   TIMSK0 = _BV(TOIE0); /* Enable interrupt. */
   OCR0A  = 155;
}


void timer_start( int timer, int ms, void (*func)(int) )
{
   timers[ timer ].left = ms;
   timers[ timer ].func = func;
}


void timer_stop( int timer )
{
   timers[ timer ].left = 0;
}


