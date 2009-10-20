

#include "conf.h"

#include "adc.h"
#include "event.h"
#include "mod.h"
#include "pwm.h"


/**
 * @brief Initializes all the subsystems.
 */
void init (void)
{
   /* ADC. */
   adc_initIO();

   /* PWM. */
   pwm_initIO();

   /* Modules. */
   mod_init();
}


/**
 * @brief Main entry point.
 */
void main (void)
{
   init();

   for (;;) {
   }
}
