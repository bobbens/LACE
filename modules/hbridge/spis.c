

#include "spis.h"

#include <avr/interrupt.h>


/*
 * Pin and Port information.
 */
#define DDR_SPI   DDRB  /* SPI Peripheral DDR. */
#define DD_SCK    PB5   /* Slave SCK pin. */
#define DD_MISO   PB4   /* Slave MISO pin. */
#define DD_MOSI   PB3   /* Slave MOSI pin. */
#define DD_SS     PB2   /* Slave SS pin. */


/**
 * @brief Initialize the SPI interface.
 */
void spis_init (void)
{
   volatile char io_reg;

   /* Set MISO output, all others input */
   DDR_SPI |= _BV(DD_MISO);
   DDR_SPI &= ~(_BV(DD_SCK) | _BV(DD_MOSI) | _BV(DD_SS));

   /* Enable SPI */
   SPCR     = _BV(SPE) | _BV(SPIE);

   /* Clear the SPDF bit. */
   io_reg   = SPSR;
   io_reg   = SPDR;
}


/**
 * @brief SPI Serial Transfer complete.
 */
ISR( SPI_STC_vect )
{
   volatile char c;

   c = SPDR;
}



