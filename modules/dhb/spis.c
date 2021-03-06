

#include "spis.h"

#include <avr/interrupt.h>
#include <util/crc16.h>

#include "ioconf.h"
#include "motors.h"
#include "hbridge.h"
#include "comm.h"
#include "current.h"
#include "sched.h"


/*
 * Pin and Port information.
 */
#define DDR_SPI   DDRB  /* SPI Peripheral DDR. */
#define DD_SCK    PB5   /* Slave SCK pin. */
#define DD_MISO   PB4   /* Slave MISO pin. */
#define DD_MOSI   PB3   /* Slave MOSI pin. */
#define DD_SS     PB2   /* Slave SS pin. */


#define SPIS_CMD_RESET() \
spis_cmd_func = spis_cmd_start; \
spis_pos = 0


static uint8_t spis_pos = 0;
uint8_t spis_crc = 0;
uint8_t spis_buf[5];


/*
 * SPI Slave Command functions.
 */
static void spis_cmd_start (void);
static void spis_cmd_version (void);
static void spis_cmd_modeset (void);
static void spis_cmd_motorset (void);
static void spis_cmd_motorget (void);
static void spis_cmd_current (void);
static void (*spis_cmd_func)(void) = spis_cmd_start;


/**
 * @brief Initialize the SPI interface.
 */
inline void spis_init (void)
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
   SPIS_CMD_RESET();
}


static void spis_cmd_start (void)
{
   uint8_t c = SPDR;
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
   else {
      switch (c) {
         case DHB_CMD_VERSION:
            spis_cmd_func = spis_cmd_version;
            break;

         case DHB_CMD_MODESET:
            spis_cmd_func = spis_cmd_modeset;
            break;

         case DHB_CMD_MOTORSET:
            spis_cmd_func = spis_cmd_motorset;
            break;

         case DHB_CMD_MOTORGET:
            sched_flags |= SCHED_SPIS_PREP_MOTORGET;
            spis_cmd_func = spis_cmd_motorget;
            break;

         case DHB_CMD_CURRENT:
            sched_flags |= SCHED_SPIS_PREP_CURRENT;
            spis_cmd_func = spis_cmd_current;
            break;

         default:
            SPIS_CMD_RESET();
            LED0_ON();
            return;
      }
      spis_pos = 0;
      spis_crc = _crc_ibutton_update( 0, c );
   }

   /* Echo. */
   SPDR = c;
}


/**
 * @brief Handles SPI for the version command.
 */
static void spis_cmd_version (void)
{
   SPDR = DHB_VERSION;
   SPIS_CMD_RESET();
   LED0_OFF();
}


/**
 * @brief Handles SPI for the mode set command.
 */
static void spis_cmd_modeset (void)
{
   uint8_t c = SPDR;
   if (spis_pos < 1) {
      /* Fill buffer. */
      spis_buf[ spis_pos++ ] = c;
      /* Echo recieved. */
      SPDR     = c;
      /* Update CRC. */
      spis_crc = _crc_ibutton_update( spis_crc, c );
   }
   else {
      /* Check CRC. */
      if (c != spis_crc) {
         SPIS_CMD_RESET();
         LED0_ON();
         return;
      }
      /* Set mode. */
      motor_mode( spis_buf[0] );
      /* Clear command. */
      SPIS_CMD_RESET();
      LED0_OFF();
   }
}


/**
 * @brief Handles SPI for the motor set command.
 */
static void spis_cmd_motorset (void)
{
   int16_t mota, motb;
   uint8_t c = SPDR;
   /* Still processing input. */
   if (spis_pos < 4) {
      /* Fill buffer. */
      spis_buf[ spis_pos++ ] = c;
      /* Echo recieved. */
      SPDR     = c;
      /* Update CRC. */
      spis_crc = _crc_ibutton_update( spis_crc, c );
   }
   /* Handle command. */
   else {
      /* Check CRC. */
      if (c != spis_crc) {
         SPIS_CMD_RESET();
         LED0_ON();
         return;
      }
      /* Prepare arguments. */
      mota  = (spis_buf[0]<<8) + spis_buf[1]; 
      motb  = (spis_buf[2]<<8) + spis_buf[3];
      /* Set motor. */
      motor_set( mota, motb );
      /* Clear command. */
      SPIS_CMD_RESET();
      LED0_OFF();
   }
}


/**
 * @brief Handles SPI for the motor get command.
 */
static void spis_cmd_motorget (void)
{
   if (spis_pos < 4) {
      SPDR  = spis_buf[ spis_pos ];
      spis_pos++;
   }
   else {
      SPDR  = spis_crc;
      /* Clear command. */
      SPIS_CMD_RESET();
      LED0_OFF();
   }
}


/**
 * @brief Handles SPI for the current command.
 */
static void spis_cmd_current (void)
{
   if (spis_pos < 4) {
      SPDR  = spis_buf[ spis_pos ];
      spis_pos++;
   }
   else {
      SPDR  = spis_crc;
      /* Clear command. */
      SPIS_CMD_RESET();
      LED0_OFF();
   }
}


/**
 * @brief SPI Serial Transfer complete.
 *
 *          1  2  3  4  5
 *    0  1  2  3  4  5  6     n
 * M 80 CM X1 X2 X3 X4 X5... CRC
 * S 00 80 CM Y1 Y2 Y3 Y4... CRC
 */
ISR( SPI_STC_vect )
{
   spis_cmd_func();
}



