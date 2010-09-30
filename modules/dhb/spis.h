

#ifndef _SPIS_H
#  define _SPIS_H


#include <stdint.h>


extern uint8_t spis_crc;
extern uint8_t spis_buf[5];


/**
 * @brief Initializes the SPI as slave.
 */
inline void spis_init (void);


#endif /* _SPIS_H */
