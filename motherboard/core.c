

#include "conf.h"

#include "adc.h"
#include "event.h"
#include "mod.h"
#include "pwm.h"
#include "servo.h"
#include "spim.h"
#include "timer.h"
#include "comm.h"
#include "mod/dhb.h"

#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#define ABS(x)    ((x)>0)?(x):-(x)


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
   comm_init(); /* Communication. */
   event_init(); /* Events. */

   /*
    * Optional subsystems.
    */
   /*servo_init();*/ /* Servo motors on TIMER1. */
   timer_init(); /* Timer infrastructure on TIMER0. */

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


static void fsm_search (void);
static void fsm_run (void);
#define FSM_SEARCH      1
#define FSM_RUN         2
static int fsm_state    = 0;
static int fsm_adc      = 0;
static int fsm_turn     = 0;
static int16_t fsm_speed = 0;
static uint16_t fsm_adcBuf[2];
static void fsm( event_t *evt )
{
   switch (evt->type) {
      case EVENT_TYPE_TIMER:
         if (evt->timer.timer == 0) {
            LED0_TOGGLE();
            timer_start( 0, 500, NULL );
         }
         else if (evt->timer.timer == 1) {

            if (fsm_state == FSM_SEARCH)
               fsm_search();
            else if (fsm_state == FSM_RUN)
               fsm_run();
         }
         break;

      case EVENT_TYPE_ADC:
         fsm_adcBuf[ fsm_adc ]  = ADCL;
         fsm_adcBuf[ fsm_adc ] += ADCH<<8;
         /*printf( "adc %d: %u", fsm_adc, fsm_adcBuf[ fsm_adc ] );*/
         fsm_adc = 1-fsm_adc;
         adc_start( fsm_adc );
         break;

      default:
         break;
   }
}


static void fsm_search (void)
{
   uint16_t dist;

   fsm_speed = 20;
   if (fsm_turn > 0) {
      dhb_target( 1, fsm_speed, -fsm_speed );
      fsm_turn = -1;
   }
   else if (fsm_turn == 0) {
      dhb_target( 1, -fsm_speed, fsm_speed );
      fsm_turn = -1;
   }

   /* Calculate distance as average of both sensors. */
   dist = (fsm_adcBuf[0] + fsm_adcBuf[1]) >> 1;

   if ((dist < 300) && (fsm_turn < 0))
      fsm_state = FSM_RUN;

   timer_start( 1, 500, NULL );
}


static void fsm_run (void)
{
   uint16_t dist;
   int16_t speed;
   int16_t diff;

   /* Calculate distance as average of both sensors. */
   dist = (fsm_adcBuf[0] + fsm_adcBuf[1]) >> 1;

   if (dist > 400) {
      /* Choose direction to turn. */
      if (fsm_adcBuf[0] > fsm_adcBuf[1])
         fsm_turn = 1;
      else
         fsm_turn = 0;

      /* Change state. */
      fsm_state = FSM_SEARCH;
      fsm_speed = -50;
   }
   else {
      speed = (512 - dist)/3;
      diff  = fsm_speed - speed;
      diff  = ABS(diff);
      if (diff > 20)
         fsm_speed = speed;
      if (fsm_speed > 64)
         fsm_speed = 64;
   }

   /* Adjust speed. */
   dhb_target( 1, fsm_speed, fsm_speed );

   timer_start( 1, 500, NULL );
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

   /* Initialize modules. */
   dhb_init( 1 );

   /* Start timer. */
   timer_start( 0, 500, NULL );

   /* Start fsm. */
   fsm_state   = FSM_SEARCH;
   timer_start( 1, 500, NULL );
   adc_start( fsm_adc );

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


