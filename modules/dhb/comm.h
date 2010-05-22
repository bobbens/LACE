

#ifndef COMM_H
#  define COMM_H

/* Init */
void comm_init (void);
void USART0_Init( unsigned int baud );
void USART0_SendByte(uint8_t Data);
uint8_t USART0_ReceiveByte(void);
uint8_t comm_transmit( char maxlen, char* buf );


#endif /* COMM_H */

