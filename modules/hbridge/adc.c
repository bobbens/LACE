
#include "global.h"

#include <avr/io.h>

/* Initializes ADC subsystem. */
void adc_init(void)
{
	/* Select reference voltage:
    * AVCC with external capacitor at AREF pin */
	ADMUX  = _BV(REFS0);

   /* Set up main register. */
	ADCSRA = _BV(ADEN) | /* Enable ADC. */
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); /* 128 prescaler. */
}

/* ADC single conversion routine. */
uint16_t adc_start_conversion(uint8_t channel)
{
	uint16_t adc_value;

	/* set ADC channel */
	ADMUX = (ADMUX & 0xF0) | channel;

	/* Start conversion */
	ADCSRA |= _BV(ADSC);
	while (bit_is_set(ADCSRA, ADSC));

   /* 10 bit right sided value. */
	adc_value = ADCL;   
	adc_value += (ADCH<<8);

	return adc_value;
}

