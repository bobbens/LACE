

#include "conf.h"

#include "adc.h"
#include "event.h"
#include "mod.h"
#include "pwm.h"
#include "servo.h"
#include "spim.h"
#include "timer.h"
#include "comm.h"
#include "fsm.h"

#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>


/**
 * @brief Initializes all the subsystems.
 */
static void init (void)
{
   /* Disable some subsystems. */
   PRR = _BV(PRTWI) | /* Disable the TWI. */
         _BV(PRTIM2) | /* Disable the Timer 2. */
         _BV(PRTIM0) | /* Disable the Timer 0. */
         _BV(PRUSART1) | /* Disable the USART 1. */
         _BV(PRTIM1) | /* Disable the Timer 1. */
         _BV(PRSPI) | /* Disable the SPI. */
         _BV(PRUSART0) | /* Disable the USART 0. */
         _BV(PRADC); /* Disable ADC. */

   /* Initialize the LED. */
   LED0_INIT();

   /*
    * Initialize subsystems.
    */
   mod_init(); /* Modules. */
   adc_init(); /* ADC. */
   pwm_init(); /* PWM. */
   comm_init(); /* Communication. */
   event_init(); /* Events. */

   /*
    * Optional subsystems.
    */
   /*servo_init();*/ /* Servo motors on TIMER1. */
   timer_init(); /* Timer infrastructure on TIMER0. */

   /* Set sleep mode. */
   set_sleep_mode( SLEEP_MODE_IDLE );

   /* Enable interrupts. */
   sei();
}


/**
 * @brief Main entry point.
 */
int main (void)
{
   event_t evt;

   /* Disable watchdog timer since it doesn't always get reset on restart. */
   wdt_disable();     

   /* Initialize the MCU. */
   init();

   /* Online. */
   printf( "Exocore Apollo online...\n" );

   /* Start fsm. */
   fsm_start();

   for (;;) {
      /* Atomic test to see if has anything to do. */
      cli();

      /* Handle events. */
      while (event_poll(&evt)) {
         sei(); /* Reenable interrupts. */
         fsm( &evt );
         cli(); /* Disable for next check. */
      }

      /* Atomic sleep as specified on the documentation. */
      sleep_enable();
      sei();
      sleep_cpu();
      sleep_disable();
   }
}


