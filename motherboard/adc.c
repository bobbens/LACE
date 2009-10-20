

#include "ioconf.h"

#include "adc.h"


void adc_initIO (void)
{
   DDRA = 0x00; /* All as read only. */
}
