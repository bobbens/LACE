

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
 * @brief Initializes the SPI as master.
 */
void i2cm_init( uint8_t freq );


/**
 * @brief Exits the SPI subsystem.
 */
void i2cm_exit (void);


/**
 * @brief Starts a transmission.
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


void i2cm_recieveLen( int len );


/**
 * @brief Starts the spi transmission on the port.
 *
 *    @param port Port to start transmitting data on.
 */
void i2cm_end (void);


/**
 * @brief Begins a transmission.
 *
 *    @param data Data to transmit.
 *    @param len Length of the data to transmit.
 */
void i2cm_transmit( uint8_t addr, const char *data, int len );


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
 * @brief Checks to see if the SPI module is idle.
 *
 *    @return 1 if idle.
 */
int i2cm_idle (void);


/**
 * @brief Gets the incoming buffer.
 *
 *    @param[out] len Length of incoming buffer.
 *    @return The incoming buffer.
 */
char* i2cm_inbuf( int *len );


/**
 * @brief Gets the outgoing buffer.
 *
 *    @param[out] len Length of outgoing buffer.
 *    @return The outgoing buffer.
 */
char* i2cm_outbuf( int *len );


#endif /* _SPIM_H */

