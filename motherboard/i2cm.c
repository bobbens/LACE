

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
static uint8_t i2c_ok            = 0; /**< Set to 1 if transmission was completed successfully. */


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


void i2cm_start( uint8_t addr, int rw )
{
   /* Clear buffers. */
   i2c_pos     = 0;
   i2c_len     = 1; /* Length is 1 because of the address+rw byte. */
   i2c_ok      = 0;
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


/**
 * @brief Signal handler indicating transfer complete.
 */
ISR( TWI_vect )
{
   event_t evt;

   switch (TWSR) {

      /* Writing. */
      case I2C_START: /* START has been transmitted  */
      case I2C_REP_START: /* Repeated START has been transmitted */
         i2c_pos = 0; /* Write address + rw */
         /* Purpose fallthrough. */
      case I2C_MTX_ADR_ACK: /* SLA+W has been tramsmitted and ACK received */
      case I2C_MTX_DATA_ACK: /* Data byte has been tramsmitted and ACK received */
         if (i2c_pos < i2c_len) { /* Still have stuff to send. */
            TWDR = i2c_buf[ i2c_pos++ ];                /* Send next byte. */
            TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT); /* Keep running like before. */
         }
         else { /* Send STOP after last byte. */
            i2c_ok = 1; /* It went OK. */
            TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | /* Keep running like before. */
                   _BV(TWSTO); /* Send STOP. */
         }
         break;

      /* Reading. */
      case I2C_MRX_DATA_ACK: /* Data byte has been received and ACK tramsmitted */
         i2c_buf[ i2c_pos++ ] = TWDR;  /* Store new byte. */
         /* Purpose fallthrough. */
      case I2C_MRX_ADR_ACK: /* SLA+R has been tramsmitted and ACK received */
         if (i2c_pos < i2c_len-1) { /* Read normally until last byte to NAK. */
            TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | /* Keep running like before. */
                   _BV(TWEA); /* ACK the byte. */
         }
         else { /* Send NAK after recieving. */
            TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT); /* Keep running like before. */
         }
         break;

      /* Finish read. */
      case I2C_MRX_DATA_NACK: /* Data byte has been received and NACK tramsmitted */
         i2c_ok = 1; /* It went OK. */
         i2c_buf[ i2c_pos ] = TWDR; /* Store last byte. */
         TWCR = _BV(TWEN) | /* Keep i2c enabled. */
                _BV(TWINT) | /* Clear interrupt. */
                _BV(TWSTO); /* Send STOP. */
         break;

      /* Arbitration. */
      case I2C_ARB_LOST: /* Arbitration lost */
         TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | /* Keep running like before. */
                _BV(TWSTA); /* Send (RE)START condition. */
         break;

      /* Errors. */
      case I2C_MTX_ADR_NACK: /* SLA+W has been tramsmitted and NACK received */
      case I2C_MRX_ADR_NACK: /* SLA+R has been tramsmitted and NACK received */
      case I2C_MTX_DATA_NACK: /* Data byte has been tramsmitted and NACK received */
      case I2C_STATE_NONE: /* No relevant state information available. */
      case I2C_BUS_ERROR: /* Bus error due to an illegal START or STOP condition */
      default:
         i2c_state = TWSR; /* Store state. */
         TWCR = _BV(TWEN); /* Keep i2c enabled. */

         /* Send end of transmission event. */
         evt.type      = EVENT_TYPE_I2C;
         evt.i2c.address = i2c_buf[0] >> 1;
         evt.i2c.rw    = i2c_buf[0] & 0x01;
         evt.i2c.ok    = i2c_ok;
         event_push( &evt );
         break;
   }
}

