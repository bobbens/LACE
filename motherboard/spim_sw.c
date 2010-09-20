

#include "conf.h"

#include "spim.h"
#include "event.h"

#include <avr/interrupt.h>


/*
 * Buffers.
 */
static int spi_clk   = 0;
static int spi_pos   = 0;
static char spi_en   = 0;
static char spi_spdr = 0x00;
static char spi_outBuf[ SPI_BUFFER_LEN ]; /**< Outgoing SPI master buffer. */
static char spi_inBuf[ SPI_BUFFER_LEN ]; /**< Incoming SPI master buffer. */
static volatile int  spi_len    = 0; /**< Current write SPI master buffer. */
static volatile int  spi_inPos  = 0; /**< Incoming SPI master buffer position. */
static volatile int  spi_outPos = 0; /**< Outgoing SPI master buffer position. */
static volatile int  spi_port   = 0; /**< Currently selected SPI port. */


/**
 * @brief Initializes the SPI perpipheral as master.
 */
void spim_init (void)
{
   /* Enable Timer 1. */
   PRR   &= ~_BV(PRTIM2);

   /* CTC Mode
    *
    *               f_clk
    * f_ctc = -----------------
    *         2 * N * (1 + TOP)
    *
    *    f_clk
    * ------------- - 1 = TOP
    * f_ctc * 2 * N
    *
    *  f_clk   = 20e6
    *  f_ctc   = 8e3 (1 kHz SPI)
    *  N       = 8
    *  TOP     = 155.25 ~= 155
    *
    *  Since we generate 2 interrupts each cycle, we'll need it to be at twice.
    *
    *  TOP*2 = 76
    */
   TCCR2A = _BV(WGM21); /* CTC mode. */
   TCCR2B = _BV(CS21) | _BV(CS20); /* 8 prescaler. */
   TCNT2  = 0; /* Clear timer. */
   OCR2A  = 155;
   OCR2B  = 0;
   TIMSK2 &= ~_BV(OCIE2A); /* Disable interrupt. */

   /* Configure pins. */
   SPI_DDR &= ~_BV(SPI_MISO); /* MISO as input. */
   SPI_DDR |= _BV(SPI_MOSI) | _BV(SPI_SCK) | _BV(SPI_SS); /* MOSI and SCK as output. */

   /* Defaults. */
   SPI_PORT |= _BV(SPI_MOSI);
   SPI_PORT &= ~_BV(SPI_SCK);

   /* Initialize slave SS pins. */
   MOD1_SS_DDR |= _BV(MOD1_SS_P);
   MOD2_SS_DDR |= _BV(MOD2_SS_P);

   /* Unselect slaves. */
   MOD1_SS_PORT |=  _BV(MOD1_SS_P);
   MOD2_SS_PORT |=  _BV(MOD2_SS_P);

   /* Start out stopped. */
   spi_en = 0;
}


/**
 * @brief Exits the SPI subsystem.
 */
void spim_exit (void)
{
   PRR  |= _BV(PRTIM2);
}


/**
 * @brief Signal handler indicating transfer complete.
 *
 * M 80 CM X1 X2 X3 X4 X5... CRC
 * S 00 80 CM Y1 Y2 Y3 Y4... CRC
 */
ISR( TIMER2_COMPA_vect )
{
   event_t evt;

   /* Handle clock down. */
   if (spi_clk) {
      SPI_PORT &= ~_BV(SPI_SCK);
      spi_clk = 0;
   }

   /* Handle the bits. */
   if (spi_pos < 8) {
      /* Write bit. */
      if (spi_spdr & 0x80)
         SPI_PORT |= _BV(SPI_MOSI);
      else
         SPI_PORT &= ~_BV(SPI_MOSI);

      /* Read bit. */
      spi_spdr <<= 1;
      if (SPI_PIN & _BV(SPI_MISO))
         spi_spdr |= 0x01;

      /* Set clock up. */
      SPI_PORT |= _BV(SPI_SCK);
      spi_clk = 1;

      spi_pos++;
      return;
   }
   spi_pos = 0;

   /* Get last character. */
   spi_inBuf[ spi_outPos-1 ] = spi_spdr;

   /* Finished, so we break. */
   if (spi_outPos >= spi_len) {
      /* Unselect slaves. */
      MOD1_SS_PORT |=  _BV(MOD1_SS_P);
      MOD2_SS_PORT |=  _BV(MOD2_SS_P);

      /* End transmission event. */
      evt.type      = EVENT_TYPE_SPI;
      evt.spi.port  = spi_port;
      event_push( &evt );

      /* Disable SPI. */
      TIMSK2 &= ~_BV(OCIE2A); /* Disable interrupt. */
      spi_en = 0;
   }

   /* Get ready for next write. */
   spi_spdr = spi_outBuf[ spi_outPos++ ];
}


void spim_transmitStart (void)
{
   /* Clear buffers. */
   spi_outPos  = 1;
   spi_inPos   = 0;
   spi_len     = 0;
}


void spim_transmitChar( char ch )
{
   if (spi_len >= SPI_BUFFER_LEN)
      return;
   spi_outBuf[ spi_len++ ] = ch;
}


void spim_transmitString( const char *data, int len )
{
   int i, n;
   n = len;
   if (spi_len + n > SPI_BUFFER_LEN)
      n = SPI_BUFFER_LEN - spi_len;
   for (i=0; i<n; i++) {
      spi_outBuf[ spi_len++ ] = data[i];
   }
}


void spim_transmitEnd( int port )
{
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
   spi_en = 1;

   /* Write first byte. */
   spi_spdr  = spi_outBuf[0];

   /* Writing first bit. */
   spi_pos = 0;
   spi_clk = 0;

   /* Start timer. */
   TCNT2  = 0; /* Clear timer. */
   TIMSK2 = _BV(OCIE2A); /* Enable interrupt. */
}


/**
 * @brief Transmits data.
 */
void spim_transmit( int port, const char *data, int len )
{
   spim_transmitStart();
   spim_transmitString( data, len );
   spim_transmitEnd( port );
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
 * @brief Reads a character.
 */
char spim_readChar (void)
{
   return spi_inBuf[ spi_inPos++ ];
}


/**
 * @brief SPI module is idle.
 */
int spim_idle (void)
{
   return !spi_en;
}


char* spim_inbuf( int *len )
{
   *len = sizeof( spi_inBuf );
   return spi_inBuf;
}


char* spim_outbuf( int *len )
{
   *len = sizeof( spi_outBuf );
   return spi_outBuf;
}

