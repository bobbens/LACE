

#ifndef COMM_H
#  define COMM_H


#ifdef DEBUG
#  define dprintf( x, args... )      printf( x, ## args )
#else /* DEBUG */
#  define dprintf( x, args... )      do {} while(0)
#endif /* DEBUG */


/* Init */
void comm_init (void);
void USART0_Init( unsigned int baud );
void USART0_SendByte(uint8_t Data);
uint8_t USART0_ReceiveByte(void);
uint8_t comm_transmit( char maxlen, char* buf );


#endif /* COMM_H */

