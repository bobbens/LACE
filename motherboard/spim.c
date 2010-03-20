

#include "conf.h"

#include "spim.h"
#include "event.h"

#include <avr/interrupt.h>


/*
 * Buffers.
 */
static char spi_outBuf[ SPI_BUFFER_LEN ]; /**< Outgoing SPI master buffer. */
static char spi_inBuf[ SPI_BUFFER_LEN ]; /**< Incoming SPI master buffer. */
static volatile int  spi_len    = 0; /**< Current write SPI master buffer. */
static volatile int  spi_inPos  = 0; /**< Incoming SPI master buffer position. */
static volatile int  spi_outPos = 0; /**< Outgoing SPI master buffer position. */
static volatile int  spi_port   = 0; /**< Currently selected SPI port. */


/**
 * @brief Initializes the SPI perpihperal as master.
 */
void spim_init (void)
{
   /*volatile char io_reg;*/

   /* Configure pins. */
   SPI_DDR &= ~_BV(SPI_MISO); /* MISO as input. */
   SPI_DDR |= _BV(SPI_MOSI) | _BV(SPI_SCK) | _BV(SPI_SS); /* MOSI and SCK as output. */

   /* Initialize slave SS pins. */
   MOD1_SS_DDR |= _BV(MOD1_SS_P);
   MOD2_SS_DDR |= _BV(MOD2_SS_P);

   /* Unselect slaves. */
   MOD1_SS_PORT |=  _BV(MOD1_SS_P);
   MOD2_SS_PORT |=  _BV(MOD2_SS_P);

   /* Configure the SPI. */
   SPCR     =  /*_BV(SPE) |*/ /* Enable SPI. */
               _BV(SPIE) | /* Enable interrupts. */
               _BV(MSTR) | /* Master mode set */
               /*_BV(SPR0);*/ /* fck/8 */
               _BV(SPR1) | _BV(SPR0); /* fck/128 */

      
   /* Clear the SPDF bit. */
   /*io_reg   = SPSR;
   io_reg   = SPDR;*/
}


/**
 * @brief Signal handler indicating transfer complete.
 */
ISR( SPI_STC_vect )
{
   event_t evt;

   /* Get last character. */
   spi_inBuf[ spi_outPos-1 ] = SPDR;

   /* Finished, so we break. */
   if (spi_outPos >= spi_len) {
      /* Unselect slaves. */
      MOD1_SS_PORT |=  _BV(MOD1_SS_P);
      MOD2_SS_PORT |=  _BV(MOD2_SS_P);
      /* Disable SPI. */
      SPCR         &= ~_BV(SPE);

      /* End transmission event. */
      evt.type      = EVENT_TYPE_SPI;
      evt.spi.port  = spi_port;
      event_push( &evt );
   }

   /* Get ready for next write. */
   SPDR = spi_outBuf[ spi_outPos++ ];
}


/**
 * @brief Transmits data.
 */
void spim_transmit( int port, const char *data, int len )
{
   int i;

   /* Clear buffers. */
   spi_outPos = 1;
   spi_inPos  = 0;
   spi_len    = len;

   /* Fill output buffer. */
   for (i=1; i<len; i++)
      spi_outBuf[i] = data[i];

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
   
      default:
         MOD1_SS_PORT |= _BV(MOD1_SS_P);
         MOD2_SS_PORT |= _BV(MOD2_SS_P);
         break;
   }

   /* Enable SPI. */
   SPCR |= _BV(SPE);

   /* Write first byte. */
   SPDR  = data[0];
}


/**
 * @brief Reads data.
 */
int spim_read( char *data, int max )
{
   int i;

   i = 0;
   while ((i < max) && (spi_inPos < spi_len))
      data[ i++ ] = spi_inBuf[ spi_inPos++ ];

   return i;
}


/**
 * @brief SPI module is idle.
 */
int spim_idle (void)
{
   return (!(SPCR & _BV(SPE)));
}


