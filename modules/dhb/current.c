
#include "current.h"

#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>


static uint16_t current_channel = 0;
static uint16_t current_buffer[2];


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
uint16_t current_get( int channel )
{
   return current_buffer[channel];
}


/**
 * @brief ADC conversion complete.
 */
ISR( ADC_vect )
{
   /* Save data. */
   current_buffer[current_channel] = (ADCH<<8) + ADCL;
   current_channel = 1 - current_channel;

   /* Disable ADC. */
   ADCSRA &= ~_BV(ADEN);
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

   /* Enable ADC. */
   ADCSRA |= _BV(ADEN);

   /* Start conversion */
   ADCSRA |= _BV(ADSC);
}


