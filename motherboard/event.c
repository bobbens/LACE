

#include "conf.h"

#include <stdio.h>

#include "event.h"


static volatile event_t event_stack[ EVENT_STACK_SIZE ];
static volatile int event_top = 0;
static volatile event_callback_t event_callbacks[ EVENT_TYPE_MAX ];


void event_init (void)
{
   int i;

   event_top = 0;

   /* Clear events. */
   for (i=0; i<EVENT_TYPE_MAX; i++)
      event_callbacks[ i ] = NULL;
}


void event_setCallback( event_type_t type, event_callback_t func )
{
   event_callbacks[ type ] = func;
}


void event_push( event_t *evt )
{
   /* Abort adding event. */
   if (event_top >= EVENT_STACK_SIZE)
      return;

   /* Check callback. */
   if (event_callbacks[ evt->type ] != NULL) {
      /* Run callback and see if need to copy over. */
      if (event_callbacks[ evt->type ]( evt ))
         return;
   }

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

