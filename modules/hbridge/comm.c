/** @file */

#include "global.h"

#include <stdio.h>
#include <inttypes.h>
#include <avr/io.h>

#include "comm.h"

/**
 * @brief Define baud rate 
 *
 * Speed to comunicate with host, probably a PC.
 */
#define USART0_BAUD 9600ul

/** 
 * @brief A macro that evaluates UBRR content
 *
 * It is suposed that normal speed is desired.
 */
#define USART0_UBBR0_VALUE ((F_CPU/(USART0_BAUD<<4))-1)


/**
 * @brief Prototypes.
 */
static int uart_putchar(char c, FILE *stream);


/**
 * @brief STDOUT for printf.
 */
static FILE mystdout = FDEV_SETUP_STREAM( uart_putchar, NULL,
                                          _FDEV_SETUP_WRITE );


#if 0
/**
 * @brief Handles recieving data.
 *
 *    @param len Length of the recieved data.
 *    @param buf Data with the recieved data.
 */
static void comm_receive( u08 len, u08 *buf )
{
   (void)len;
   (void)buf;
}
#endif

/**
 * @brief Handles transmitting data.
 *
 *    @param maxlen Maximum length of the transmit buffer.
 *    @param[out] buf Buffer with data to transmit.
 *    @return Amount of data written to the buffer.
 */
uint8_t comm_transmit( char maxlen, char* buf )
{
	unsigned char i;
	for(i=0;i<maxlen;i++)
		USART0_SendByte(*buf++);
   return i;
}


/**
 * @brief Puts a character on the IO.
 */
static int uart_putchar(char c, FILE *stream)
{
   (void) stream;
   USART0_SendByte( c );
   return 0;
}



/**
 * @brief Initializes USART0
 *
 *  Detailed description: USART stuff!! 
 */
void USART0_Init( unsigned int baud )
{
   /* Set baud rate */
   UBRR0H = (unsigned char)(baud>>8);

   UBRR0L = (unsigned char)baud;

   /* Enable receiver and transmitter */
   UCSR0B = (1<<RXEN0)|(1<<TXEN0);

   /* Set frame format: 8data, 2stop bit */
   UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

/**
 * @brief Sends a byte via RS-232
 *
 */
void USART0_SendByte(uint8_t Data)
{

   // Wait if a byte is being transmitted
   while( !(UCSR0A & (1<<UDRE0)) );

   // Transmit data
   UDR0 = Data; 

}

/**
 * @brief Waits till a byte is received
 *
 */
uint8_t USART0_ReceiveByte()
{

   // Wait until a byte has been received
   while( !(UCSR0A & ( 1<<RXC0 )) ) ;

   // Return received data
   return UDR0;

}

/**
 * @brief Initializes the communication subsystem.
 *
 */
void comm_init (void)
{
   /* Set up USART. */
   USART0_Init( USART0_UBBR0_VALUE );

   /* Set up printf. */
   stdout = &mystdout;
}
