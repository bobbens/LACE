

#include "conf.h"

#include "adc.h"


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
   /* Select reference voltage:
    * AVCC with external capacitor at AREF pin */
   ADMUX  = _BV(REFS0);

   /* Set up main register. */
   ADCSRA = _BV(ADEN) | /* Enable ADC. */
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); /* 128 prescaler. */
}
