

#ifndef _RS232_H
#  define _RS232_H


#include <stdint.h>


/*
 * Modes for USART speeds (20 MHz crystal with U2Xn = 1).
 */
/*        SPEED         UBRR     ERROR */
#define USART_2_4k      1041  /*  0.0% */
#define USART_4_8k      520   /*  0.0% */
#define USART_9_6k      259   /*  0.2% */
#define USART_14_4k     173   /* -0.2% */
#define USART_19_2k     129   /*  0.2% */
#define USART_28_8k     86    /* -0.2% */
#define USART_38_4k     64    /*  0.2% */
#define USART_57_6k     42    /*  0.9% */
#define USART_78_8k     32    /* -1.4% */
#define USART_115_2k    21    /* -1.4% */
#define USART_230_4k    10    /* -1.4% */
#define USART_250k      9     /*  0.0% */
#define USART_500k      4     /*  0.0% */


/*
 * USART 0 Functionality.
 */
/**
 * @brief Initializes the USART0 to do the RS232 protocol.
 *
 *    @param baud Baud rate should be taken from the defines.
 */
void rs232_init0( uint16_t baud );
/**
 * @brief Checks to see if there's data in the RX buffer.
 *
 *    @return 0 if there isn't data in the buffer, >0 if there is.
 */
int rs232_status0 (void);
/**
 * @brief Gets the next character in the RX buffer.
 *
 *    @return The next character in the RX buffer.
 */
char rs232_get0 (void);
/**
 * @brief Puts a character in the TX buffer.
 *
 *    @param c Character to put in the TX buffer.
 */
void rs232_put0( char c );
/**
 * @brief Sets a callback function when an rs232 byte is recieved.
 */
void rs232_setRecv0( void (*recvFunc)(char) );



#endif /* _RS232_H */
