

#include "conf.h"

#include <string.h>
#include <stdio.h>

#include "fsm.h"
#include "timer.h"
#include "mod/dhb.h"
#include "event_cust.h"
#include "spim.h"


static int fsm_action = 0;


void fsm_start (void)
{
   dhb_init(1);
   timer_start( 0, 250, NULL ); /* 250 ms start delay. */
}


void fsm( event_t *evt )
{
   uint16_t cura, curb;
   int16_t fbka, fbkb;

   switch (evt->type) {
      case EVENT_TYPE_TIMER:
         if (evt->timer.timer == 0) {
            dhb_mode( 1, DHB_MODE_FBKS );
            timer_start( 2, 250, NULL );
         }
         else if (evt->timer.timer == 1) {
            if (fsm_action)
               dhb_feedback( 1 );
            else
               dhb_current( 1 );
            /*
            int n;
            char *inbuf;
            inbuf = spim_inbuf( &n );
            printf( "%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                  inbuf[0], inbuf[1], inbuf[2], inbuf[3],
                  inbuf[4], inbuf[5], inbuf[6], inbuf[7],
                  inbuf[8] );
            */
            fsm_action = 1 - fsm_action;
            LED0_TOGGLE();
         }
         else if (evt->timer.timer == 2) {
            dhb_target( 1, 70, 40 );
            timer_start( 1, 100, NULL );
         }
         break;

      case EVENT_TYPE_CUSTOM:
         if (evt->custom.id == EVENT_CUST_DHB_FEEDBACK) {
            if (evt->custom.data == 0)
               printf( "DHB Feedback CRC error\n" );
            else {
               dhb_feedbackValue( 1, &fbka, &fbkb );
               printf( "fbk %d %d\n", fbka, fbkb );
            }
            timer_start( 1, 100, NULL );
         }
         else if (evt->custom.id == EVENT_CUST_DHB_CURRENT) {
            if (evt->custom.data == 0)
               printf( "DHB Current CRC error\n" );
            else {
               dhb_currentValue( 1, &cura, &curb );
               printf( "cur %u %u\n", cura, curb );
            }
            timer_start( 1, 100, NULL );
         }
         break;

      default:
         break;
   }
}


