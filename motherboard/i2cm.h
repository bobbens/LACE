

#ifndef _SPIM_H
#  define _SPIM_H


#include "ioconf.h"


/* Set frequency.
 *
 *                      CPU Clock
 *    SCL freq = --------------------------
 *               16 + 2 * (TWBR) * 4 ^ TWPS
 *
 * For 20 MHz:
 *
 *    357 kHz ==> TWBR = 5
 *                TWPS = 1
 *    100 kHz ==> TWBR = 23
 *                TWPS = 1
 *
 * For 16 MHz
 *
 *    400 kHz ==> TWBR = 3
 *                TWPS = 1
 *    100 kHz ==> TWBR = 18
 *                TWPS = 1
 *
 * Pullups should be (for 400 kHz):
 *
 * Vcc = 5 V
 * Cb  = 10 pF + bus capitance
 *
 *    Vcc - 0.4 V                1000 ns
 *    -----------  <  pullup  <  -------
 *        3 ma                     Cb
 *
 * For a 5 v 10 pF bus:
 *
 *    1533 ohm < pullup < 50 kohm
 *
 *  A good value is 10k.
 */
#if F_CPU == 20000000UL
#define I2C_FREQ_400K   0x05
#define I2C_FREQ_100k   0x23
#elif F_CPU == 16000000UL
#define I2C_FREQ_400K   0x03
#define I2C_FREQ_100k   0x18
#endif /* F_CPU */


#define I2C_WRITE    0 /**< Do I2C write. */
#define I2C_READ     1 /**< Do I2C read. */


/**
 * @brief Initializes the I2C as master.
 *
 * @usage i2cm_init( I2C_FREQ_400K );
 *
 *    @param freq Frequency macro to set at.
 */
void i2cm_init( uint8_t freq );


/**
 * @brief Exits the I2C subsystem.
 */
void i2cm_exit (void);


/**
 * @brief Starts a transmission.
 *
 * You have to set the data to send with i2cm_transmitChar and
 *  i2cm_transmitString. To actually begin sending you have to finalize with
 *  i2cm_end.
 *
 *    @param addr Address to write to.
 *    @param rw Should be either I2C_WRITE or I2C_READ depeding on whether it
 *              should be a read or a write.
 */
void i2cm_start( uint8_t addr, int rw );


/**
 * @brief Adds a char to the output buffer.
 *
 *    @param ch Char to add.
 */
void i2cm_transmitChar( char ch );


/**
 * @brief Adds a string to the output buffer.
 *
 *    @param data Data to add.
 *    @param len Length of the data to add.
 */
void i2cm_transmitString( const char *data, int len );


/**
 * @brief Starts the i2c communication.
 */
void i2cm_end (void);


/**
 * @brief Begins a transmission.
 *
 *    @param addr Address to send data to.
 *    @param data Data to transmit.
 *    @param len Length of the data to transmit.
 */
void i2cm_transmit( uint8_t addr, const char *data, int len );


/**
 * @brief Begins a reception.
 *
 *    @param addr Address to get data from.
 *    @param len Maximum data length to get (slave can make shorter).
 */
void i2cm_recieve( uint8_t addr, int len );


/**
 * @brief Gets the data read during the transmission.
 *
 *    @param data Data to read (should be allocated already).
 *    @param max Length of the buffer being filled.
 *    @return Amount read.
 */
int i2cm_read( char *data, int max );


/**
 * @brief Checks to see if the I2C module is idle.
 *
 *    @return 1 if idle.
 */
int i2cm_idle (void);


#endif /* _SPIM_H */

