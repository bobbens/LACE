

#include "spis.h"

#include <avr/interrupt.h>
#include <util/crc16.h>
#include <stdio.h>

#include "ioconf.h"
#include "motors.h"
#include "hbridge.h"
#include "comm.h"
#include "current.h"


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
   volatile char io_reg;

   /* Enable power. */
   PRR &= ~_BV(PRSPI);

   /* Set MISO output, all others input */
   DDR_SPI |= _BV(DD_MISO);
   DDR_SPI &= ~(_BV(DD_SCK) | _BV(DD_MOSI) | _BV(DD_SS));

   /* Enable SPI */
   SPCR     = _BV(SPE) | _BV(SPIE);

   /* Clear the SPDF bit. */
   io_reg   = SPSR;
   io_reg   = SPDR;

   /* Reset the entire communication thingy. */
   spis_cmd = DHB_CMD_NONE;
   spis_pos = 0;
}


/**
 * @brief SPI Serial Transfer complete.
 */
ISR( SPI_STC_vect )
{
   int16_t mota, motb;
   char c = SPDR;

   /* Not processing a command currently. */
   if (spis_cmd == DHB_CMD_NONE) {

      /* Handle package start. */
      if (spis_pos==0) {
         if (c == 0x80)
            spis_pos = 1;
         else {
            SPDR = 0;
            return;
         }
      }

      /* Handle command. */
      else if (spis_pos==1) {
         spis_cmd = c;
         spis_pos = 0;
         spis_crc = _crc_ibutton_update( 0, c );
      }

      /* Echo. */
      SPDR = c;
   }

   /* Handle data. */
   else {

      switch (spis_cmd) {
         case DHB_CMD_VERSION:
            SPDR     = DHB_VERSION;
            spis_cmd = DHB_CMD_NONE;
            spis_pos = 0;
            break;

         case DHB_CMD_MODESET:
            if (spis_pos < 1) {
               /* Fill buffer. */
               spis_buf[ spis_pos++ ] = c;
               /* Update CRC. */
               spis_crc = _crc_ibutton_update( spis_crc, c );
               /* Echo recieved. */
               SPDR     = c;
            }
            else {
               /* Check CRC. */
               if (c != spis_crc) {
                  spis_cmd = DHB_CMD_NONE;
                  spis_pos = 0;
                  break;
               }
               /* Set mode. */
               motor_mode( spis_buf[0] );
               /* Clear command. */
               spis_cmd = DHB_CMD_NONE;
               spis_pos = 0;
            }
            break;

         case DHB_CMD_MOTORSET:
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
               if (c != spis_crc) {
                  spis_cmd = DHB_CMD_NONE;
                  spis_pos = 0;
                  break;
               }
               /* Prepare arguments. */
               mota  = spis_buf[0]<<8;
               mota += spis_buf[1];
               motb  = spis_buf[2]<<8;
               motb += spis_buf[3];
               /* Set motor. */
               motor_set( mota, motb );
               /* Clear command. */
               spis_cmd = DHB_CMD_NONE;
               spis_pos = 0;
            }
            break;

         case DHB_CMD_MOTORGET:
            if (spis_pos == 0) {
               motor_get( &mota, &motb );
               spis_buf[0] = mota >> 8;
               spis_buf[1] = mota & 0xFF;
               spis_buf[2] = mota >> 8;
               spis_buf[3] = mota & 0xFF;
               spis_crc    = _crc_ibutton_update( spis_crc, spis_buf[0] );
               SPDR        = spis_buf[0];
               spis_pos    = 1;
            }
            else if (spis_pos < 4) {
               SPDR = spis_buf[ spis_pos ];
               spis_crc = _crc_ibutton_update( spis_crc, spis_buf[ spis_pos ] );
               spis_pos++;
            }
            else {
               SPDR  = spis_crc;
               /* Clear command. */
               spis_cmd = DHB_CMD_NONE;
               spis_pos = 0;
            }
            break;

         case DHB_CMD_CURRENT:
            if (spis_pos == 0) {
               mota = current_get(0);
               motb = current_get(1);
               spis_buf[0] = mota >> 8;
               spis_buf[1] = mota & 0xFF;
               spis_buf[2] = mota >> 8;
               spis_buf[3] = mota & 0xFF;
               spis_crc    = _crc_ibutton_update( spis_crc, spis_buf[0] );
               SPDR        = spis_buf[0];
               spis_pos    = 1;
            }
            else if (spis_pos < 4) {
               SPDR = spis_buf[ spis_pos ];
               spis_crc = _crc_ibutton_update( spis_crc, spis_buf[ spis_pos ] );
               spis_pos++;
            }
            else {
               SPDR  = spis_crc;
               /* Clear command. */
               spis_cmd = DHB_CMD_NONE;
               spis_pos = 0;
            }
            break;

         default:
            SPDR     = 0x00;
            spis_cmd = DHB_CMD_NONE;
            spis_pos = 0;
            break;
      }
   }
}



