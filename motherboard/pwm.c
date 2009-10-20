

#include "conf.h"

#include "pwm.h"


void pwm_initIO (void)
{
   /* Set pins as output. */
   PWM0_DDR |= _BV(PWM0A) | _BV(PWM0B);
   PWM1_DDR |= _BV(PWM1A) | _BV(PWM1B);
   PWM2_DDR |= _BV(PWM2A) | _BV(PWM2B);
}


