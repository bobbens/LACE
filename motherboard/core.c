

#include "conf.h"

#include "adc.h"
#include "event.h"
#include "mod.h"
#include "pwm.h"
#include "servo.h"
#include "rs232.h"
#include "spim.h"

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>


static uint8_t sched_flags = 0;


/**
 * @brief Initializes all the subsystems.
 */
static void init (void)
{
   /* Disable some subsystems. */
   PRR = _BV(PRTWI); /* Disable the TWI. */

   /* Initialize the LED. */
   LED0_INIT();

   /*
    * Initialize subsystems.
    */
   spim_init(); /* SPI Master. */
   adc_init(); /* ADC. */
   pwm_init(); /* PWM. */
   /*mod_init();*/ /* Modules. */

   /* Set sleep mode. */
   set_sleep_mode( SLEEP_MODE_IDLE );

   /* Set up watchdog timer. */
#if 0
   wdt_reset(); /* Just in case. */
   wdt_enable( WDTO_15MS );
#endif

   /* Enable interrupts. */
   sei();
}


#if 0
static int recv_pos        = 0;
static uint16_t recv_pwm   = 0;
/**
 * @brief Handles data reception.
 */
static void recv( char c )
{
   /* Check for packet start. */
   if (recv_pos==0) {
      if (c == 0x80) {
         rs232_put0( c );
         recv_pos = 1;
      }
      return;
   }
   else if (recv_pos==1) {
      rs232_put0( c );
      recv_pwm = c<<8;
   }
   else if (recv_pos==2) {
      rs232_put0( c );
      recv_pwm += c;
      servo_pwm1A( recv_pwm );
      recv_pos = 0;
   }
}
#endif


/**
 * @brief Main entry point.
 */
int main (void)
{
   uint8_t flags;

   /* Disable watchdog timer since it doesn't always get reset on restart. */
   wdt_disable();     

   /* Initialize the MCU. */
   init();
   /*servo_init1();*/

   char buf[] = { 0x80, 0x00, 0x40, 0x00, 0x40 };
   while (1) {
      spim_transmit( 1, buf, sizeof(buf) );
      _delay_ms( 500. );
      LED0_TOGGLE();
   }

   for (;;) {
      /* Atomic test to see if has anything to do. */
      cli();
      if (sched_flags != 0) {

         /* Atomic store temporary flags and reset real flags in case we run a bit late. */
         flags = sched_flags;
         sched_flags = 0;
         sei(); /* Restart interrupts. */

         /* Run scheduler. */
         /*sched_run( flags );*/
      }
      /* Sleep. */
      else {
         /* Atomic sleep as specified on the documentation. */
         sleep_enable();
         sei();
         sleep_cpu();
         sleep_disable();
      }
   }
}


