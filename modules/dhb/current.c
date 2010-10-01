
#include "current.h"

#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>


static uint8_t current_channel = 0;
int8_t current_buffer[4];


/**
 * @brief Initializes ADC subsystem.
 */
inline void current_init (void)
{
   /* Enable power. */
   PRR &= ~_BV(PRADC);

	/* Select reference voltage:
    * ARef, if we choose AVCC, ARef will be shorted. */
	ADMUX  = 0x00; /* _BV(REFS0); */

   /* We only need ADC6 and ADC7 which aren't IO. */
   DIDR0 = 0x00;

   /* Set up main register. */
	ADCSRA = _BV(ADEN) | /* Enable ADC. */
            _BV(ADIE) | /* Enable interrupts. */
   /* Prescaler and timing
    *
    * MCU is at 20 MHz
    * Current sampling is done at 333 Hz
    *
    * 128 prescaler -> 156.25 kHz
    *
    * First sample needs 27 cycles, next need 13 cycles:
    *    First sample -> 156.25/27 =  5.787 kHz
    *    Later sample -> 156.25/13 = 12.019 kHz
    */
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); /* 128 prescaler. */
   ADCSRB = 0; /* No autotrigger. */
}


/**
 * @brief ADC conversion complete.
 */
ISR( ADC_vect )
{
   /* Save data. */
   if (current_channel) {
      current_buffer[2] = ADCH;
      current_buffer[3] = ADCL;
      current_channel   = 0;
   }
   else {
      current_buffer[0] = ADCH;
      current_buffer[1] = ADCL;
      current_channel   = 1;
   }
}


/**
 * @brief ADC single conversion routine.
 */
inline void current_startSample (void)
{  
   /* Set ADC channel */
   if (current_channel == 0)
      ADMUX = 0x06;
   else
      ADMUX = 0x07;

   /* Start conversion */
   ADCSRA |= _BV(ADSC);
}


