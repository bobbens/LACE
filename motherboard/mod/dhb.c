

#include "mod.h"

#include "dhb.h"

#include "spim.h"
#include "mod_def.h"

#include <util/crc16.h>
#include <stdio.h>


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

   /* Check if empty. */
   mod            = mod_get( port );
   if (mod->id != MODULE_ID_NONE)
      return -1;

   /* Turn port on. */
   mod_on( port );

   /* Set data. */
   mod->id        = MODULE_ID_DHB;
   mod->version   = 1;
   mod->on        = 1;

   return 0;
}


void dhb_exit( int port )
{
   module_t *mod;

   /* Check if valid. */
   mod = mod_get( port );
   if (mod->id != MODULE_ID_DHB)
      return;

   mod->id        = MODULE_ID_NONE;
   mod->version   = 0;
   mod->on        = 0;
   mod_off( port );
}


static int dhb_send( int port, char cmd, char *data, int len )
{
   int i;
   char crc;
   module_t *mod;

   /* Check module. */
   mod = mod_get( port );
   if (mod->id != MODULE_ID_DHB)
      return -1;

   /* Check sending. */
   if (!spim_idle())
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
   char data[1];
   data[0] = mode;
   return dhb_send( port, DHB_CMD_MODESET, data, sizeof(data) );
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
   return dhb_send( port, DHB_CMD_MOTORSET, data, sizeof(data) );
}


int dhb_feedback( uint16_t *m0, uint16_t *m1 )
{
   *m0 = 0;
   *m1 = 0;

   return 0;
}


