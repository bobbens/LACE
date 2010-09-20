
#include "current.h"

#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>


static uint16_t current_channel = 0;
static uint8_t current_buffer[4];


/**
 * @brief Initializes ADC subsystem.
 */
void current_init (void)
{
   /* Enable power. */
   PRR &= ~_BV(PRADC);

	/* Select reference voltage:
    * AVCC with external capacitor at AREF pin */
	ADMUX  = _BV(REFS0);

   /* We only need ADC6 and ADC7 which aren't IO. */
   DIDR0 = 0x00;

   /* Set up main register. */
	ADCSRA = _BV(ADEN) | /* Enable ADC. */
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); /* 128 prescaler. */
   ADCSRB = 0; /* No autotrigger. */
}


/**
 * @brief Gets the current data buffer of the current level.
 */
void current_get( uint8_t *out )
{
   out[0] = current_buffer[0];
   out[1] = current_buffer[1];
   out[2] = current_buffer[2];
   out[3] = current_buffer[3];
}


/**
 * @brief ADC conversion complete.
 */
ISR( ADC_vect )
{
   /* Save data. */
   current_buffer[current_channel*2+0] = ADCH;
   current_buffer[current_channel*2+1] = ADCL;
   current_channel = 1 - current_channel;
}


/**
 * @brief ADC single conversion routine.
 */
void current_startSample (void)
{  
   /* Set ADC channel */
   if (current_channel == 0)
      ADMUX = (ADMUX & 0xF0) | 6;
   else
      ADMUX = (ADMUX & 0xF0) | 7;

   /* Start conversion */
   ADCSRA |= _BV(ADSC);
}


