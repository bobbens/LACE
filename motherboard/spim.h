

#ifndef _SPIM_H
#  define _SPIM_H


/**
 * @brief Initializes the SPI as master.
 */
void spim_init (void);


/**
 * @brief Starts a transmission.
 */
void spim_transmitStart (void);


/**
 * @brief Adds a char to the output buffer.
 *
 *    @param ch Char to add.
 */
void spim_transmitChar( char ch );


/**
 * @brief Adds a string to the output buffer.
 *
 *    @param data Data to add.
 *    @param len Length of the data to add.
 */
void spim_transmitString( const char *data, int len );


/**
 * @brief Starts the spi transmission on the port.
 *
 *    @param port Port to start transmitting data on.
 */
void spim_transmitEnd( int port );


/**
 * @brief Begins a transmission.
 *
 *    @param data Data to transmit.
 *    @param len Length of the data to transmit.
 */
void spim_transmit( int port, const char *data, int len );


/**
 * @brief Gets the data read during the transmission.
 *
 *    @param data Data to read (should be allocated already).
 *    @param max Length of the buffer being filled.
 *    @return Amount read.
 */
int spim_read( char *data, int max );


/**
 * @brief Checks to see if the SPI module is idle.
 *
 *    @return 1 if idle.
 */
int spim_idle (void);


#endif /* _SPIM_H */

