

#include "conf.h"

#include "adc.h"
#include "event.h"

#include <avr/interrupt.h>


static volatile int adc_channel     = 0; /**< Current ADC channel it's on. */


/**
 * @brief Initializes the ADC IO pins.
 */
static __inline void adc_initIO (void)
{
   /* All ADC pins as read only. */
   DDRA = 0x00;
}


void adc_init (void)
{
   /* Enable power. */
   PRR &= ~_BV(PRADC);

   /* Select reference voltage:
    * AVCC with external capacitor at AREF pin */
   ADMUX  = _BV(REFS0);

   /* Disable digital IO. */
   DIDR0  = 0xFF;

   /* Set up main register. */
   ADCSRA = /*_BV(ADEN) |*/ /* Enable ADC. */
            _BV(ADIE) | /* Use interrupts. */
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); /* 128 prescaler. */
   ADCSRB = 0; /* No autotrigger. */
}


void adc_exit (void)
{
   /* Disable ADC. */
   ADCSRA = 0;

   /* Disable power. */
   PRR   |= _BV(PRADC);

   /* Enable digital IO. */
   DIDR0  = 0;
}


/**
 * @brief ADC conversion complete.
 */
ISR( ADC_vect )
{
   event_t evt;

   /* Set up the event. */
   evt.type          = EVENT_TYPE_ADC;
   evt.adc.channel   = adc_channel;

   /* Push the event. */
   event_push( &evt );

   /* Disable ADC. */
   ADCSRA &= ~_BV(ADEN);
}


/**
 * @brief Starts a conversion on a channel.
 */
void adc_start( int channel )
{
   /* Save the ADC channel. */
   adc_channel = channel;

   /* Set ADC channel */
   ADMUX = (ADMUX & 0xF0) | adc_channel;

   /* Enable ADC. */
   ADCSRA |= _BV(ADEN);

   /* Start conversion */
   ADCSRA |= _BV(ADSC);
}
