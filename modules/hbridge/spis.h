

#ifndef _SPIM_H
#  define _SPIM_H


/**
 * @brief Initializes the SPI as master.
 */
void spis_init (void);


/**
 * @brief Begins a transmission.
 *
 *    @param data Data to transmit.
 *    @param len Length of the data to transmit.
 */
void spim_transmit( int port, char *data, int len );


/**
 * @brief Gets the data read during the transmission.
 *
 *    @param data Data to read (should be allocated already).
 *    @param max Length of the buffer being filled.
 *    @return Amount read.
 */
int spim_read( char *data, int max );


#endif /* _SPIM_H */
