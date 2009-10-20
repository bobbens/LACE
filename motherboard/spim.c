

#include "ioconf.h"

#include "spim.h"
#include "buf_ring.h"


#define RING_BUFFER_LEN       64


typedef struct ring_buffer_s {
   char buf[ RING_BUFFER_LEN ];
   int start;
   int end;
} ring_buffer_t;


static ring_buffer_t spi_out = { .start = 0, .end = 0 };
static ring_buffer_t spi_in  = { .start = 0, .end = 0 };


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
   buf->end = (buf->end + 1) % RING_BUFFER_LEN;
   buf->buf[ buf->end ] = c;
}
/**
 * @brief Gets a character from the ring buffer.
 */
static __inline char ring_get( ring_buffer_t *buf )
{
   buf->start = (buf->start + 1) % RING_BUFFER_LEN;
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
   /* Get last character. */
   ring_put( &spi_in, SPDR );

   /* Finished, so we break. */
   if (ring_empty( &spi_out )) {
      SPCR &= ~_BV(SPE);
   }

   /* Get ready for next write. */
   SPDR = ring_get( &spi_out );
}


void spim_transmit( char *data, int len )
{
   int i;

   /* Clear buffers. */
   ring_clear( &spi_in );
   ring_clear( &spi_out );

   /* Fill output buffer. */
   for (i=0; i<len; i++)
      ring_put( &spi_out, data[i] );

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


