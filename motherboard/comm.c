

#include "comm.h"

#include "conf.h"

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"


static FILE mystdout = FDEV_SETUP_STREAM( (int(*)(char,FILE*)) uart_putc, NULL, _FDEV_SETUP_WRITE );


void comm_init (void)
{
   /* Enable the uart0 peripheral. */
   PRR   &= ~_BV(PRUSART0);

   /* Start up the module. */
   uart_init( UART_BAUD_SELECT(COMM_BAUD,F_CPU) );
   stdout = &mystdout;
}
