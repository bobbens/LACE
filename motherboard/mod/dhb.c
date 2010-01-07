

#include "dhb.h"

#include "mod.h"


int dhb_init( int port )
{
   /* Turn port on. */
   mod_on( port );

   /* Make sure it's detected. */
   if (!mod_detect( port ))
      return -1;

   /* Send a packet and see if we recieve it. */
}


void dhb_mode( dhb_mode_t mode )
{
   char buf[2];
   buf[0]      = DHB_CMD_MODE;
   buf[1]      = mode;
}


void dhb_target( uint16_t t0, uint16_t t1 )
{
   char buf[5];
   buf[0]      = DHB_CMD_TARGET;
   buf[1]      = t0<<8
   buf[2]      = t0;
   buf[3]      = t1<<8;
   buf[4]      = t1;
}


void dhb_feedback( uint16_t *m0, uint16_t *m1 )
{
   *m0 = 0;
   *m1 = 0;
}


#endif /* _MOD_HBRIDGE_H */
