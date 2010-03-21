

#include "conf.h"

#include <stdio.h>

#include "event.h"


static volatile event_t event_stack[ EVENT_STACK_SIZE ];
static volatile int event_top = 0;


void event_init (void)
{
   event_top = 0;
}


void event_push( event_t *evt )
{
   /* Abort adding event. */
   if (event_top >= EVENT_STACK_SIZE)
      return;

   /* Copy over. */
   event_stack[ event_top++ ] = *evt;
}


int event_poll( event_t *evt )
{
   int i;

   /* Make sure there's events left. */
   if (event_top <= 0)
      return 0;

   /* Copy event. */
   *evt = event_stack[0];

   /* Move down. */
   i = 1;
   while (i<event_top) {
      event_stack[i-1] = event_stack[i];
      i++;
   }
   event_top--;

   /* Notify there's an event. */
   return 1;
}

