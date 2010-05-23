

#include "conf.h"

#include "fsm.h"
#include "timer.h"
#include "adc.h"
#include "mod/dhb.h"

#include <stdint.h>
#include <string.h>


#define ABS(x)    ((x)>0)?(x):-(x)
#define FSM_INIT        1
#define FSM_SEARCH      2
#define FSM_RUN         3
#define FSM_ERR         4
static int fsm_state    = 0;
static int fsm_adc      = 0;
static int fsm_turn     = 0;
static int16_t fsm_speed = 0;
static uint16_t fsm_adcBuf[2];


/*
 * Prototypes
 */
static void fsm_search (void);
static void fsm_run (void);
static void fsm_init( event_t *evt );


void fsm_start (void)
{
   fsm_state   = FSM_INIT;
   dhb_init( 1 ); /* Initialize the peripheral. */
   timer_start( 0, 250, NULL ); /* 250 ms start delay. */
}


static void fsm_init( event_t *evt )
{
   switch (evt->type) {
      case EVENT_TYPE_SPI:
         fsm_state   = FSM_SEARCH;
         timer_start( 0, 500, NULL );
         timer_start( 1, 500, NULL );
         adc_start( fsm_adc );
         break;

      case EVENT_TYPE_TIMER:
         dhb_mode( 1, DHB_MODE_FBKS );
         break;

      default:
         break;
   }
}


void fsm( event_t *evt )
{
   /* Start with the init loop. */
   if (fsm_state == FSM_INIT) {
      fsm_init( evt );
      return;
   }
   else if (fsm_state == FSM_ERR) {
      LED0_ON();
      return;
   }

   /* Handle normal events. */
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


