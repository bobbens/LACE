

#include "mod.h"

#include "dhb.h"

#include "spim.h"
#include "mod.h"
#include "mod_def.h"
#include "event.h"
#include "event_cust.h"

#include <util/crc16.h>
#include <stdio.h>


/*
 * Internal usage variables.
 */
static int16_t dhb_var_feedback[MOD_PORT_NUM*2]; /**< Current speed value. */
static uint16_t dhb_var_current[MOD_PORT_NUM*2]; /**< Current current value. */


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

static int dhb_feedback_callback( event_t* evt )
{
   char *inbuf;
   int len;
   event_t new_evt;

   /* Store value. */
   inbuf = spim_inbuf( &len );
   dhb_var_feedback[(evt->spi.port-1)*2+0] = (inbuf[3]<<8) + inbuf[4];
   dhb_var_feedback[(evt->spi.port-1)*2+1] = (inbuf[5]<<8) + inbuf[6];

   /* Generate event. */
   new_evt.type         = EVENT_TYPE_CUSTOM;
   new_evt.custom.id    = EVENT_CUST_DHB_FEEDBACK;
   new_evt.custom.data  = evt->spi.port;
   event_push( &new_evt );
   event_setCallback( EVENT_TYPE_SPI, NULL ); /* Disable callback. */
   return 1; /* Destroy event. */
}
int dhb_feedback( int port )
{
   char data[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
   int ret = dhb_send( port, DHB_CMD_MOTORGET, data, sizeof(data) );
   if (ret == 0)
      event_setCallback( EVENT_TYPE_SPI, dhb_feedback_callback );
   return ret;
}
void dhb_feedbackValue( int port, int16_t *mota, int16_t *motb )
{
   *mota = dhb_var_feedback[(port-1)*2+0];
   *motb = dhb_var_feedback[(port-1)*2+1];
}


static int dhb_current_callback( event_t* evt )
{
   char *inbuf;
   int len;
   event_t new_evt;

   /* Store value. */
   inbuf = spim_inbuf( &len );
   dhb_var_current[(evt->spi.port-1)*2+0] = (inbuf[3]<<8) + inbuf[4];
   dhb_var_current[(evt->spi.port-1)*2+1] = (inbuf[5]<<8) + inbuf[6];

   /* Generate event. */
   new_evt.type         = EVENT_TYPE_CUSTOM;
   new_evt.custom.id    = EVENT_CUST_DHB_CURRENT;
   new_evt.custom.data  = evt->spi.port;
   event_push( &new_evt );
   event_setCallback( EVENT_TYPE_SPI, NULL ); /* Disable callback. */
   return 1; /* Destroy event. */
}
int dhb_current( int port )
{
   char data[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
   int ret = dhb_send( port, DHB_CMD_CURRENT, data, sizeof(data) );
   if (ret == 0)
      event_setCallback( EVENT_TYPE_SPI, dhb_current_callback );
   return ret;
}
void dhb_currentValue( int port, uint16_t *mota, uint16_t *motb )
{
   *mota = dhb_var_current[(port-1)*2+0];
   *motb = dhb_var_current[(port-1)*2+1];
}


