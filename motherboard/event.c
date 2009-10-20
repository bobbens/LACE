

#include "conf.h"

#include "event.h"


static event_t event_stack[ EVENT_STACK_SIZE ];
static int event_top = 0;


void event_push( event_t *evt )
{
   /* Abort adding event. */
   if (event_top >= EVENT_STACK_SIZE)
      return;

   /* Copy over. */
   event_stack[ event_top ] = *evt;
}


int event_poll( event_t *evt )
{
   int i;

   /* Make sure there's events left. */
   if (event_stack[0].type == EVENT_TYPE_NONE)
      return 0;

   /* Copy event. */
   *evt = event_stack[0];

   /* Move down. */
   i = 1;
   while ((i<EVENT_STACK_SIZE) && (event_stack[i].type != EVENT_TYPE_NONE)) {
      event_stack[i-1] = event_stack[i];
   }

   /* Notify there's an event. */
   return 1;
}

