

#ifndef _ADC_H
#  define _ADC_H


/**
 * @brief Initializes the ADC subsystem.
 */
void adc_init (void);


/**
 * @brief Starts an ADC conversion.
 */
void adc_start( int channel );


#endif /* _ADC_H */

