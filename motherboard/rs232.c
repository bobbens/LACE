

#include "conf.h"

#include "rs232.h"

#include <avr/interrupt.h>


typedef struct ring_buffer_s {
   char buf[ USART0_RING_BUFFER_LEN ];
   int head;
   int tail;
} ring_buffer_t;


static ring_buffer_t usart0_out = { .head = 0, .tail = 0 };
static ring_buffer_t usart0_in  = { .head = 0, .tail = 0 };


/**
 * @brief Clears a ring buffer.
 */
static __inline void ring_clear( ring_buffer_t *buf )
{
   buf->head  = 0;
   buf->tail  = 0;
}
/**
 * @brief Adds a character to the ring buffer.
 */
static __inline void ring_put( ring_buffer_t *buf, char c )
{
   buf->tail = (buf->tail + 1) & (RING_BUFFER_LEN-1);
   buf->buf[ buf->tail ] = c;
}
/**
 * @brief Gets a character from the ring buffer.
 */
static __inline char ring_get( ring_buffer_t *buf )
{
   buf->head = (buf->head + 1) & (RING_BUFFER_LEN-1);
   return buf->buf[ buf->head ];
}
/**
 * @brief Checks to see if the ring buffer is empty.
 */
static __inline int ring_empty( ring_buffer_t *buf )
{
   return (buf->head == buf->tail);
}
/**
 * @brief Checks to see if the ring buffer is full.
 */
static __inline int ring_full( ring_buffer_t *buf )
{
   return (buf->tail+1 == buf->head);
}


void rs232_init0( uint16_t baud )
{
   /* Set baud rate */
   UBRR0H = (unsigned char)(baud>>8);
   UBRR0L = (unsigned char)baud;

   /* Set peripheral flags. */
   UCSR0A = _BV(U2X0); /* Double transmission speed. */
   UCSR0B = _BV(RXEN0) | _BV(TXEN0) | /* Enable reciever and transmitter. */
            _BV(RXCIE0); /* Enable recieve interrupt. */
   UCSR0C = _BV(USBS0) | _BV(UCSZ00); /* Frame format: 8 data, 2 stop bit */
}


int rs232_status0 (void)
{
   return !ring_empty( &usart0_in );
}


char rs232_get0 (void)
{
   return ring_get( &usart0_in );
}


void rs232_put0( char c )
{
   ring_put( &usart0_out, c );
   UCSR0B   |= _BV(UDRIE0); /* Tell to update output buffer with data if possible. */
}


/**
 * @brief USART0 recieve data interrupt.
 */
ISR( SIG_USART_RECV )
{
   /* Add new data. */
   if (!ring_full( &usart0_in )) {
      ring_put( &usart0_in, UDR0 );
   }
   /* Out of room. */
   else {
   }
}


/**
 * @brief USART1 transfer data interrupt.
 */
ISR( SIG_USART_DATA )
{
   /* Send more data. */
   if (!ring_empty( &usart0_out )) {
      UDR0 = ring_get( &usart0_out );
   }
   /* Send finished. */
   else {
      UCSR0B &= ~_BV(UDRIE0); /* No longer care about empty output buffer. */
   }
}


