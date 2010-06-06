

#include "conf.h"

#include "i2cm.h"
#include "event.h"

#include <avr/interrupt.h>


/*
 * Buffers.
 */
static char i2c_outBuf[ SPI_BUFFER_LEN ]; /**< Outgoing SPI master buffer. */
static char i2c_inBuf[ SPI_BUFFER_LEN ]; /**< Incoming SPI master buffer. */
static volatile int  i2c_len    = 0; /**< Current write SPI master buffer. */
static volatile int  i2c_inPos  = 0; /**< Incoming SPI master buffer position. */
static volatile int  i2c_outPos = 0; /**< Outgoing SPI master buffer position. */
static volatile int  i2c_port   = 0; /**< Currently selected SPI port. */


/**
 * @brief Initializes the SPI perpipheral as master.
 */
void i2cm_init( uint8_t freq )
{
   /* Power up i2c preipheral. */
   PRR &= ~_BV(PRTWI);

   /* Set frequency. */
   TWBR = freq;

   /* Set state. */
   TWSR = 0;

   /* DIsable slave mode. */
   TWAR = 0;

   /* Enable i2c and release TWI pins. */
   TWCR = _BV(TWEN);
}


/**
 * @brief Exits the SPI subsystem.
 */
void i2cm_exit (void)
{
   /* Disable peripheral. */
   SPCR  = 0;

   /* Power down SPI peripheral. */
   PRR  |= ~_BV(PRSPI);
}


/**
 * @brief Signal handler indicating transfer complete.
 */
ISR( TWI_vect )
{
   event_t evt;

   /* Get last character. */
   i2c_inBuf[ i2c_outPos-1 ] = SPDR;

   /* Finished, so we break. */
   if (i2c_outPos >= i2c_len) {
      /* Unselect slaves. */
      MOD1_SS_PORT |=  _BV(MOD1_SS_P);
      MOD2_SS_PORT |=  _BV(MOD2_SS_P);
      /* Disable SPI. */
      SPCR         &= ~_BV(SPE);

      /* End transmission event. */
      evt.type      = EVENT_TYPE_SPI;
      evt.spi.port  = i2c_port;
      event_push( &evt );
   }

   /* Get ready for next write. */
   SPDR = i2c_outBuf[ i2c_outPos++ ];
}


void i2cm_transmitStart (void)
{
   /* Clear buffers. */
   i2c_outPos  = 1;
   i2c_inPos   = 0;
   i2c_len     = 0;
}


void i2cm_transmitChar( char ch )
{
   if (i2c_len >= SPI_BUFFER_LEN)
      return;
   i2c_outBuf[ i2c_len++ ] = ch;
}


void i2cm_transmitString( const char *data, int len )
{
   int i, n;
   n = len;
   if (i2c_len + n > SPI_BUFFER_LEN)
      n = SPI_BUFFER_LEN - i2c_len;
   for (i=0; i<n; i++) {
      i2c_outBuf[ i2c_len++ ] = data[i];
   }
}


void i2cm_transmitEnd( int port )
{
   /* Set the port. */
   i2c_port = port;
   switch (i2c_port) {
      case 1:
         MOD1_SS_PORT &= ~_BV(MOD1_SS_P);
         MOD2_SS_PORT |=  _BV(MOD2_SS_P);
         break;

      case 2:
         MOD1_SS_PORT |=  _BV(MOD1_SS_P);
         MOD2_SS_PORT &= ~_BV(MOD2_SS_P);
         break;
   
      default:
         MOD1_SS_PORT |= _BV(MOD1_SS_P);
         MOD2_SS_PORT |= _BV(MOD2_SS_P);
         break;
   }

   /* Enable SPI. */
   SPCR |= _BV(SPE);

   /* Write first byte. */
   SPDR  = i2c_outBuf[0];
}


/**
 * @brief Transmits data.
 */
void i2cm_transmit( int port, const char *data, int len )
{
   i2cm_transmitStart();
   i2cm_transmitString( data, len );
   i2cm_transmitEnd( port );
}


/**
 * @brief Reads data.
 */
int i2cm_read( char *data, int max )
{
   int i;

   i = 0;
   while ((i < max) && (i2c_inPos < i2c_len))
      data[ i++ ] = i2c_inBuf[ i2c_inPos++ ];

   return i;
}


/**
 * @brief SPI module is idle.
 */
int i2cm_idle (void)
{
   return (!(SPCR & _BV(SPE)));
}


char* i2cm_inbuf( int *len )
{
   *len = sizeof( i2c_inBuf );
   return i2c_inBuf;
}


char* i2cm_outbuf( int *len )
{
   *len = sizeof( i2c_outBuf );
   return i2c_outBuf;
}

