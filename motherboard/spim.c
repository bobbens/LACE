

#include "conf.h"

#include "spim.h"
#include "event.h"

#include <avr/interrupt.h>


typedef struct ring_buffer_s {
   char buf[ SPI_BUFFER_LEN ];
   int start;
   int end;
} ring_buffer_t;


static ring_buffer_t spi_out = { .start = 0, .end = 0 };
static ring_buffer_t spi_in  = { .start = 0, .end = 0 };
static int spi_port = 0; /**< Current port sending on. */


/**
 * @brief Clears a ring buffer.
 */
static __inline void ring_clear( ring_buffer_t *buf )
{
   buf->start  = 0;
   buf->end    = 0;
}
/**
 * @brief Adds a character to the ring buffer.
 */
static __inline void ring_put( ring_buffer_t *buf, char c )
{
   buf->end = (buf->end + 1) & (SPI_BUFFER_LEN-1);
   buf->buf[ buf->end ] = c;
}
/**
 * @brief Gets a character from the ring buffer.
 */
static __inline char ring_get( ring_buffer_t *buf )
{
   buf->start = (buf->start + 1) & (SPI_BUFFER_LEN-1);
   return buf->buf[ buf->start ];
}
/**
 * @brief Checks to see if the ring buffer is empty.
 */
static __inline int ring_empty( ring_buffer_t *buf )
{
   return (buf->start == buf->end);
}
/**
 * @brief Checks to see if the ring buffer is full.
 */
static __inline int ring_full( ring_buffer_t *buf )
{
   return (buf->end+1 == buf->start);
}


void spim_init (void)
{
   /* Configure pins. */
   SPI_DDR &= ~_BV(SPI_MISO); /* MISO as input. */
   SPI_DDR |= _BV(SPI_MOSI) | _BV(SPI_MISO); /* MOSI and SCK as output. */

   /* Configure the SPI. */
   SPCR     =  _BV(SPIE) | /* Enable interrupts. */
               _BV(MSTR) | /* Master mode set */
               _BV(SPR1) | _BV(SPR0); /* fck/128 */
}


/**
 * @brief Signal handler indicating transfer complete.
 */
ISR(SIG_SPI)
{
   event_t evt;

   /* Get last character. */
   ring_put( &spi_in, SPDR );

   /* Finished, so we break. */
   if (ring_empty( &spi_out )) {
      /* Unselect slaves. */
      MOD1_SS_PORT |=  _BV(MOD1_SS_P);
      MOD2_SS_PORT |=  _BV(MOD2_SS_P);
      /* Disable SPI. */
      SPCR         &= ~_BV(SPE);

      /* Send event. */
      evt.type      = EVENT_TYPE_SPI;
      evt.spi.port  = spi_port;
      event_push( &evt );
   }

   /* Get ready for next write. */
   SPDR = ring_get( &spi_out );
}


void spim_transmit( int port, char *data, int len )
{
   int i;

   /* Clear buffers. */
   ring_clear( &spi_in );
   ring_clear( &spi_out );

   /* Fill output buffer. */
   for (i=0; i<len; i++)
      ring_put( &spi_out, data[i] );

   /* Set the port. */
   spi_port = port;
   switch (spi_port) {
      case 1:
         MOD1_SS_PORT &= ~_BV(MOD1_SS_P);
         MOD2_SS_PORT |=  _BV(MOD2_SS_P);
         break;

      case 2:
         MOD1_SS_PORT |=  _BV(MOD1_SS_P);
         MOD2_SS_PORT &= ~_BV(MOD2_SS_P);
         break;
   }

   /* Enable SPI. */
   SPCR |= _BV(SPE);
}


int spim_read( char *data, int max )
{
   int i;

   i = 0;
   while ((i < max) && !ring_empty( &spi_in ))
      data[i++] = ring_get( &spi_in );

   return i;
}


