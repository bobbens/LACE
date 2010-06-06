

#include "conf.h"

#include "i2cm.h"
#include "event.h"

#include <avr/interrupt.h>


/*
 * I2C States.
 */
/* General I2C Master staus codes */
#define I2C_START                  0x08  /** START has been transmitted */
#define I2C_REP_START              0x10  /** Repeated START has been transmitted */
#define I2C_ARB_LOST               0x38  /** Arbitration lost */
/* I2C Master Transmitter staus codes */
#define I2C_MTX_ADR_ACK            0x18  /** SLA+W has been tramsmitted and ACK received */
#define I2C_MTX_ADR_NACK           0x20  /** SLA+W has been tramsmitted and NACK received */
#define I2C_MTX_DATA_ACK           0x28  /** Data byte has been tramsmitted and ACK received */
#define I2C_MTX_DATA_NACK          0x30  /** Data byte has been tramsmitted and NACK received */
/* I2C Master Receiver staus codes  */
#define I2C_MRX_ADR_ACK            0x40  /** SLA+R has been tramsmitted and ACK received */
#define I2C_MRX_ADR_NACK           0x48  /** SLA+R has been tramsmitted and NACK received */
#define I2C_MRX_DATA_ACK           0x50  /** Data byte has been received and ACK tramsmitted */
#define I2C_MRX_DATA_NACK          0x58  /** Data byte has been received and NACK tramsmitted */
/* I2C Slave Transmitter staus codes */
#define I2C_STX_ADR_ACK            0xA8  /** Own SLA+R has been received; ACK has been returned */
#define I2C_STX_ADR_ACK_M_ARB_LOST 0xB0  /** Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned */
#define I2C_STX_DATA_ACK           0xB8  /** Data byte in TWDR has been transmitted; ACK has been received */
#define I2C_STX_DATA_NACK          0xC0  /** Data byte in TWDR has been transmitted; NOT ACK has been received */
#define I2C_STX_DATA_ACK_LAST_BYTE 0xC8  /** Last data byte in TWDR has been transmitted (TWEA = 0); ACK has been received */
/* I2C Slave Receiver staus codes */
#define I2C_SRX_ADR_ACK            0x60  /** Own SLA+W has been received ACK has been returned */
#define I2C_SRX_ADR_ACK_M_ARB_LOST 0x68  /** Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned */
#define I2C_SRX_GEN_ACK            0x70  /** General call address has been received; ACK has been returned */
#define I2C_SRX_GEN_ACK_M_ARB_LOST 0x78  /** Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned */
#define I2C_SRX_ADR_DATA_ACK       0x80  /** Previously addressed with own SLA+W; data has been received; ACK has been returned */
#define I2C_SRX_ADR_DATA_NACK      0x88  /** Previously addressed with own SLA+W; data has been received; NOT ACK has been returned */
#define I2C_SRX_GEN_DATA_ACK       0x90  /** Previously addressed with general call; data has been received; ACK has been returned */
#define I2C_SRX_GEN_DATA_NACK      0x98  /** Previously addressed with general call; data has been received; NOT ACK has been returned */
#define I2C_SRX_STOP_RESTART       0xA0  /** A STOP condition or repeated START condition has been received while still addressed as Slave */
/* I2C Miscellaneous codes. */
#define I2C_STATE_NONE             0xF8  /** No relevant state information available; TWINT = 0 */
#define I2C_BUS_ERROR              0x00  /** Bus error due to an illegal START or STOP condition */


/*
 * Buffers.
 */
static char i2c_buf[ SPI_BUFFER_LEN ]; /**< I2C buffer (rw) */
static volatile int  i2c_len     = 0; /**< Length of the buffer to write or read. */
static volatile int  i2c_pos     = 0; /**< Position within the buffer. */
static int  i2c_state            = I2C_STATE_NONE; /**< Current I2C stat. */
static uint8_t i2c_twcr          = 0; /**< Temporary variable to store the TWCR before starting up. */


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
   TWCR  = 0;

   /* Power down SPI peripheral. */
   PRR  |= ~_BV(PRTWI);
}


/**
 * @brief Signal handler indicating transfer complete.
 */
ISR( TWI_vect )
{
#if 0
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
#endif
}


void i2cm_start( uint8_t addr, int rw )
{
   /* Clear buffers. */
   i2c_pos     = 0;
   i2c_len     = 1;
   i2c_state   = I2C_STATE_NONE;

   /* Start communication. */
   i2c_twcr =  _BV(TWEN) | /* Keep enabled. */
               _BV(TWIE) | /* Enable interrupt. */
               _BV(TWINT) | /* Clear interrupt flag. */
               _BV(TWSTA); /**< Send start condition. */

   /* First char must be address and rw. */
   i2c_buf[ i2c_len++ ] = (addr<<1) | rw;
}


void i2cm_transmitChar( char ch )
{
   if (i2c_len >= SPI_BUFFER_LEN)
      return;
   i2c_buf[ i2c_len++ ] = ch;
}


void i2cm_transmitString( const char *data, int len )
{
   int i, n;
   n = len;
   if (i2c_len + n > SPI_BUFFER_LEN)
      n = SPI_BUFFER_LEN - i2c_len;
   for (i=0; i<n; i++) {
      i2c_buf[ i2c_len++ ] = data[i];
   }
}


void i2cm_recieveLen( int len )
{
   i2c_len = len+1;
}


void i2cm_end (void)
{
   /* Start transmission. */
   TWCR = i2c_twcr;
}


/**
 * @brief Transmits data.
 */
void i2cm_transmit( uint8_t addr, const char *data, int len )
{
   i2cm_start( addr, I2C_WRITE );
   i2cm_transmitString( data, len );
   i2cm_end();
}


/**
 * @brief Recieves data.
 */
void i2cm_recieve( uint8_t addr, int len )
{
   i2cm_start( addr, I2C_READ );
   i2cm_recieveLen( len );
   i2cm_end();
}


/**
 * @brief Reads data.
 */
int i2cm_read( char *data, int max )
{
   int i;

   i = 1; /* First byte is the address. */
   while ((i < max) && (i2c_pos < i2c_len))
      data[ i++ ] = i2c_buf[ i2c_pos++ ];

   return i;
}


/**
 * @brief SPI module is idle.
 */
int i2cm_idle (void)
{
   return (!(TWCR & _BV(TWIE)));
}

