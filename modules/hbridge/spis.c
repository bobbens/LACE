

#include "spis.h"

#include <avr/interrupt.h>

#include "motors.h"


/*
 * Pin and Port information.
 */
#define DDR_SPI   DDRB  /* SPI Peripheral DDR. */
#define DD_SCK    PB5   /* Slave SCK pin. */
#define DD_MISO   PB4   /* Slave MISO pin. */
#define DD_MOSI   PB3   /* Slave MOSI pin. */
#define DD_SS     PB2   /* Slave SS pin. */


/*
 * The commands.
 */
#define HB_CMD_NONE     0x00
#define HB_CMD_VERSION  0x01
#define HB_CMD_MODESET  0x02
#define HF_CMD_MODEGET  0x03
#define HB_CMD_MOTORSET 0x04
#define HB_CMD_MOTORGET 0x05


static volatile char spis_cmd = 0;
static volatile int spis_pos = 0;
static volatile uint8_t spis_buf[5];


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
   volatile char c = SPDR;
   int16_t *mota, *motb;

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
      }
   }

   /* Handle data. */
   else {
      switch (spis_cmd) {
         case HB_CMD_VERSION:
            SPDR = 0x01;
            spis_cmd = 0;
            break;

         case HB_CMD_MOTORSET:
            spis_buf[ spis_pos++ ] = c;
            if (spis_pos >= 4) {
               mota = (int16_t*)(&spis_buf[0]);
               motb = (int16_t*)(&spis_buf[2]);
               motor_set( *mota, *motb );
               spis_cmd = 0;
               spis_pos = 0;
            }
            break;

         default:
            SPDR = 0x00;
            break;
      }

   }
}



