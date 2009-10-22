

#ifndef _CONF_H
#  define _CONF_H


/* USART. */
#define USART0_RING_BUFFER_LEN   64 /* Must be power of two. */


/* SPI Master. */
#define SPI_BUFFER_LEN           32 /* Must be power of two. */


/* Event. */
#define EVENT_STACK_SIZE         8 /* Size of the event stack. */


#include "ioconf.h"


#endif /* _CONF_H */
