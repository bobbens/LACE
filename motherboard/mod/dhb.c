

#include "mod.h"

#include "dhb.h"

#include "spim.h"
#include "hbridge/hbridge.h"

#include <util/crc16.h>


/*
 * Prototypes.
 */
static void dhb_send( int port, char cmd, char *data, int len );


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


static void dhb_send( int port, char cmd, char *data, int len )
{
   int i;
   char crc;

   /* Calculate CRC. */
   crc = _crc_ibutton_update( 0, cmd );
   for (i=0; i<len; i++)
      crc = _crc_ibutton_update( crc, data[i] );

   /* Send data. */
   spim_transmitStart();
   spim_transmitChar( 0x80 ); /* Header. */
   spim_transmitChar( cmd ); /* Command. */
   spim_transmitString( data, len ); /* Data. */
   spim_transmitChar( crc ); /* CRC. */
   spim_transmitEnd( port );
}


void dhb_mode( int port, char mode )
{
   dhb_send( port, HB_CMD_MODESET, &mode, sizeof(mode) );
}


void dhb_target( int port, int16_t t0, int16_t t1 )
{
   char data[4];

   /* Data. */
   data[0]  = t0>>8;
   data[1]  = t0;
   data[2]  = t1>>8;
   data[3]  = t1;

   /* Send the data. */
   dhb_send( port, HB_CMD_MOTORSET, data, sizeof(data) );
}


void dhb_feedback( uint16_t *m0, uint16_t *m1 )
{
   *m0 = 0;
   *m1 = 0;
}

