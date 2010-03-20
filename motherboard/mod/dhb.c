

#include "mod.h"

#include "dhb.h"

#include "spim.h"
#include "hbridge/hbridge.h"

#include <util/crc16.h>


int dhb_init( int port )
{
   /* Make sure it's detected. */
   /*if (!mod_detect( port ))
      return -1;*/

   /* Turn port on. */
   mod_on( port );

   /* Send a packet and see if we recieve it. */
   return 0;
}


void dhb_target( int port, int16_t t0, int16_t t1 )
{
   int i;
   char buf[7];

   /* Header. */
   buf[0]   = 0x80;
   buf[1]   = HB_CMD_MOTORSET;
   /* Data. */
   buf[2]   = t0>>8;
   buf[3]   = t0;
   buf[4]   = t1>>8;
   buf[5]   = t1;
   /* CRC. */
   buf[6]   = 0;
   for (i=1; i<6; i++)
      buf[6] = _crc_ibutton_update( buf[6], buf[i] );

   /* Send data. */
   spim_transmit( port, buf, sizeof(buf) );
}


void dhb_feedback( uint16_t *m0, uint16_t *m1 )
{
   *m0 = 0;
   *m1 = 0;
}

