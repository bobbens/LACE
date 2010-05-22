

#include "mod.h"

#include "dhb.h"

#include "spim.h"
#include "hbridge/hbridge.h"

#include <util/crc16.h>


/*
 * Prototypes.
 */
static int dhb_send( int port, char cmd, char *data, int len );


int dhb_init( int port )
{
   module_t *mod;

   /* Make sure it's detected. */
   if (!mod_detect( port ))
      return -1;

   /* Turn port on. */
   mod_on( port );

   /* Set data. */
   mod            = mod_get( port );
   mod->id        = MOD_TYPE_DHB;
   mod->version   = 1;
   mod->on        = 1;

   /* Send a packet and see if we recieve it. */
   return 0;
}


static int dhb_send( int port, char cmd, char *data, int len )
{
   int i;
   char crc;
   module_t *mod;

   /* Check module. */
   mod = mod_get( port );
   if (mod->id != MOD_TYPE_DHB)
      return -1;

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

   return 0;
}


int dhb_mode( int port, char mode )
{
   return dhb_send( port, HB_CMD_MODESET, &mode, sizeof(mode) );
}


int dhb_target( int port, int16_t t0, int16_t t1 )
{
   char data[4];

   /* Data. */
   data[0]  = t0>>8;
   data[1]  = t0;
   data[2]  = t1>>8;
   data[3]  = t1;

   /* Send the data. */
   return dhb_send( port, HB_CMD_MOTORSET, data, sizeof(data) );
}


int dhb_feedback( uint16_t *m0, uint16_t *m1 )
{
   *m0 = 0;
   *m1 = 0;

   return 0;
}

