
#include "wmp.h"

#include <string.h>

#include "i2cm.h"
#include "event.h"


/**
 * @file wmp.c
 *
 * Overview:
 *
 * wmp_start --> wmp_read --> wmp_done --> send event
 */


/*
 * Prototypes.
 */
static void wmp_err (void);
static int wmp_on( event_t *evt );
static int wmp_read( event_t *evt );
static int wmp_done( event_t *evt );
static uint16_t wmp_buf[3];


void wmp_init (void)
{
   /* Initialize I2C at 400 kHz. */
   i2cm_init( I2C_FREQ_400K );

   /* Initializes the WM+ */
   i2cm_start( 0x53, I2C_WRITE );
   i2cm_transmitChar( 0xFE );
   i2cm_transmitChar( 0x04 );
   i2cm_end();

   /* Set event for when it's on. */
   event_setCallback( EVENT_TYPE_I2C, wmp_on );
}


void wmp_exit (void)
{
   i2cm_start( 0x52, I2C_WRITE );
   i2cm_transmitChar( 0xF0 );
   i2cm_transmitChar( 0x55 );
   i2cm_end();
}


void wmp_start (void)
{
   i2cm_start( 0x52, I2C_WRITE );
   i2cm_transmitChar( 0x00 );
   i2cm_end();
   event_setCallback( EVENT_TYPE_I2C, wmp_read ); /* Start transmission. */
}


/**
 * @brief Generate an error event.
 */
static void wmp_err (void)
{
   event_t evt;
   evt.type          = EVENT_TYPE_CUSTOM;
   evt.custom.id     = WMP_EVENT_ERR;
   evt.custom.data   = 0x00;
   event_push( &evt );
}


static int wmp_on( event_t *evt )
{
   event_t wmp_evt;

   /* Failure. */
   if (!evt->i2c.ok) {
      wmp_err();
      event_setCallback( EVENT_TYPE_I2C, NULL );
      return 1;
   }

   /* Generate the on event. */
   wmp_evt.type          = EVENT_TYPE_CUSTOM;
   wmp_evt.custom.id     = WMP_EVENT_ON;
   wmp_evt.custom.data   = 0x00;
   event_push( &wmp_evt );

   return 1;
}


static int wmp_read( event_t *evt )
{
   /* Failure. */
   if (!evt->i2c.ok) {
      wmp_err();
      event_setCallback( EVENT_TYPE_I2C, NULL );
      return 1;
   }

   /* Ask for 6 bytes of data. */
   i2cm_recieve( 0x52, 6 );

   /* Next step is to process the data when we get it. */
   event_setCallback( EVENT_TYPE_I2C, wmp_done );

   /* Destroy the event. */
   return 1;
}


static int wmp_done( event_t *evt )
{
   char buf[6];
   event_t wmp_evt;

   /* Failure. */
   if (!evt->i2c.ok) {
      wmp_err();
      event_setCallback( EVENT_TYPE_I2C, NULL );
      return 1;
   }

   /* Get the data read. */
   i2cm_read( buf, sizeof(buf) );

   /* Convert to 16 bit ints. */
   wmp_buf[ WMP_YAW ]     = ((uint16_t)buf[3] << 6) + buf[0];
   wmp_buf[ WMP_PITCH ]   = ((uint16_t)buf[4] << 6) + buf[1];
   wmp_buf[ WMP_ROLL ]    = ((uint16_t)buf[5] << 6) + buf[2];

   /* Disable callback. */
   event_setCallback( EVENT_TYPE_I2C, NULL );

   /* Generate event. */
   wmp_evt.type         = EVENT_TYPE_CUSTOM;
   wmp_evt.custom.id    = WMP_EVENT_DATA;
   wmp_evt.custom.data  = ((buf[3] & 0x02) << 2) + buf[4];
   event_push( &wmp_evt );

   /* Destroy the event. */
   return 1;
}


uint16_t* wmp_data (void)
{
   return wmp_buf;
}


