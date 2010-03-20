

#include "spis.h"

#include <avr/interrupt.h>
#include <util/crc16.h>

#include "ioconf.h"
#include "motors.h"
#include "hbridge.h"


/*
 * Pin and Port information.
 */
#define DDR_SPI   DDRB  /* SPI Peripheral DDR. */
#define DD_SCK    PB5   /* Slave SCK pin. */
#define DD_MISO   PB4   /* Slave MISO pin. */
#define DD_MOSI   PB3   /* Slave MOSI pin. */
#define DD_SS     PB2   /* Slave SS pin. */


static volatile char spis_cmd    = 0;
static volatile int spis_pos     = 0;
static volatile uint8_t spis_crc = 0;
static uint8_t spis_buf[5];


/**
 * @brief Initialize the SPI interface.
 */
void spis_init (void)
{
   /*volatile char io_reg;*/

   /* Set MISO output, all others input */
   DDR_SPI |= _BV(DD_MISO);
   DDR_SPI &= ~(_BV(DD_SCK) | _BV(DD_MOSI) | _BV(DD_SS));

   /* Enable SPI */
   SPCR     = _BV(SPE) | _BV(SPIE);

   /* Clear the SPDF bit. */
   /*io_reg   = SPSR;
   io_reg   = SPDR;*/
}


/**
 * @brief SPI Serial Transfer complete.
 */
ISR( SPI_STC_vect )
{
   int16_t mota, motb;
   char c = SPDR;

   /* Not processing a command currently. */
   if (spis_cmd == HB_CMD_NONE) {

      /* Handle package start. */
      if (spis_pos==0) {
         if (c == 0x80)
            spis_pos = 1;
         else
            return;
      }

      /* Handle command. */
      else if (spis_pos==1) {
         spis_cmd = c;
         spis_pos = 0;
         spis_crc = _crc_ibutton_update( 0, c );
      }
   }

   /* Handle data. */
   else {

      switch (spis_cmd) {
         case HB_CMD_VERSION:
            SPDR     = 0x01;
            spis_cmd = HB_CMD_NONE;
            spis_pos = 0;
            break;

         case HB_CMD_MOTORSET:
            /* Still processing input. */
            if (spis_pos < 4) {
               /* Fill buffer. */
               spis_buf[ spis_pos++ ] = c;
               /* Update CRC. */
               spis_crc = _crc_ibutton_update( spis_crc, c );
               /* Echo recieved. */
               SPDR     = c;
            }
            /* Handle command. */
            else {
               /* Check CRC. */
               if (c != spis_crc)
                  break;
               /* Prepare arguments. */
               mota  = spis_buf[0]<<8;
               mota += spis_buf[1];
               motb  = spis_buf[2]<<8;
               motb += spis_buf[3];
               /* Set motor. */
               motor_set( mota, motb );
               /* Clear command. */
               spis_cmd = HB_CMD_NONE;
               spis_pos = 0;
            }
            break;

         default:
            SPDR     = 0x00;
            spis_cmd = HB_CMD_NONE;
            spis_pos = 0;
            break;
      }
   }
}



