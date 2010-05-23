/** @file */

#include "global.h"

#include <stdio.h>
#include <inttypes.h>
#include <avr/io.h>

#include "comm.h"
#include "uart.h"



/**
 * @brief STDOUT for printf.
 */
static FILE mystdout = FDEV_SETUP_STREAM( uart_putc, NULL,
                                          _FDEV_SETUP_WRITE );



/**
 * @brief Initializes the communication subsystem.
 *
 */
void comm_init (void)
{
   /* Enable power. */
   PRR &= ~_BV(PRUSART0);

   /* Set up USART. */
   uart_init( UART_BAUD_SELECT( 57600, F_CPU ) );

   /* Set up printf. */
   stdout = &mystdout;
}
