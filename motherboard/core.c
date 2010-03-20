

#include "conf.h"

#include "adc.h"
#include "event.h"
#include "mod.h"
#include "pwm.h"
#include "servo.h"
#include "rs232.h"
#include "spim.h"
#include "mod/dhb.h"

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
   mod_init(); /* Modules. */
   adc_init(); /* ADC. */
   pwm_init(); /* PWM. */

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
#if 0
   LED0_INIT();
   /* Configure module SS. */
   MOD1_SS_DDR  |= _BV(MOD1_SS_P); /* Mod1 SS as output. */
   MOD2_SS_DDR  |= _BV(MOD2_SS_P); /* Mod2 SS as output. */
   MOD1_SS_PORT |= _BV(MOD1_SS_P);
   MOD2_SS_PORT |= _BV(MOD2_SS_P);
   /* Configure pins. */
   SPI_DDR &= ~_BV(SPI_MISO); /* MISO as input. */
   SPI_DDR |= _BV(SPI_MOSI) | _BV(SPI_SCK) | _BV(SPI_SS); /* MOSI and SCK as output. */
   /* Configure the SPI. */
   SPCR     =  _BV(SPE) | /* Enable SPI. */
               /*_BV(SPIE) |*/ /* Enable interrupts. */
               _BV(MSTR) | /* Master mode set */
               _BV(SPR1) | _BV(SPR0); /* fck/128 */
   volatile char io_reg;
   io_reg = SPSR;
   io_reg = SPDR;
   _delay_ms( 500. );
   sei();
   while (1) {
      MOD1_SS_PORT &= ~_BV(MOD1_SS_P);

      SPDR = 0x80;
      while (!(SPSR & _BV(SPIF)));

      MOD1_SS_PORT |= _BV(MOD1_SS_P);
      _delay_ms( 500. );
      LED0_TOGGLE();
   }
#endif
#if 0
   dhb_init( 1 );
   _delay_ms( 500. );
   dhb_target( 1, 50, 50 );
   LED0_TOGGLE();
   while (1) {
      _delay_ms( 500. );
      LED0_TOGGLE();
   }
#endif

   uint8_t flags;

   /* Disable watchdog timer since it doesn't always get reset on restart. */
   wdt_disable();     

   /* Initialize the MCU. */
   init();
   /*servo_init1();*/

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


