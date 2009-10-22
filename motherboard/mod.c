

#include "conf.h"

#include "mod.h"

#include "spim.h"
#include "mod_def.h"

#include <avr/interrupt.h>


/**
 * @brief Internal module list.
 */
static module_t mod_data[2] = {
   { .id = MODULE_ID_NONE, .version = 0x00 },
   { .id = MODULE_ID_NONE, .version = 0x00 }
};


void mod_initIO (void)
{
   /*
    * We start them offand disable them.
    */
   /* Module 1. */
   MOD1_SS_PORT  |=  _BV(MOD1_SS_P);
   MOD1_SS_DDR   |=  _BV(MOD1_SS_P);
   MOD1_RST_DDR  |=  _BV(MOD1_RST_P);
   MOD1_RST_PORT &= ~_BV(MOD1_RST_P);
   MOD1_ON_DDR   &= ~_BV(MOD1_ON_P);
   /* Module 2. */
   MOD2_SS_PORT  |=  _BV(MOD2_SS_P);
   MOD2_SS_DDR   |=  _BV(MOD2_SS_P);
   MOD2_RST_DDR  |=  _BV(MOD2_RST_P);
   MOD2_RST_PORT &= ~_BV(MOD2_RST_P);
   MOD2_ON_DDR   &= ~_BV(MOD2_ON_P);
}


/**
 * @brief On signal handler.
 */
ISR(MOD_ON_SIG)
{
   /* Handle module 1. */
   if (mod_data[0].on != (MOD1_ON_PIN & _BV(MOD1_ON_P))) {
      mod_data[0].on       = MOD1_ON_PIN & _BV(MOD1_ON_P);
      if (mod_data[0].on) {
         /*mod_detect( 1 );*/
      }
      else {
         mod_data[0].id       = MODULE_ID_NONE;
         mod_data[0].version  = 0;
      }
   }
   /* Handle module 2. */
   if (mod_data[1].on != (MOD2_ON_PIN & _BV(MOD2_ON_P))) {
      mod_data[1].on       = MOD2_ON_PIN & _BV(MOD2_ON_P);
      if (mod_data[1].on) {
         /*mod_detect( 2 );*/
      }
      else {
         mod_data[1].id       = MODULE_ID_NONE;
         mod_data[1].version  = 1;
      }
   }
}


void mod_init (void)
{
   /* Initialize IO. */
   mod_initIO();

   /* Enable interrupts. */
   PCICR      |= _BV(MOD_ON_INT);
   MCUCR      |= /*_BV(ISC01) |*/ _BV(ISC00); /* Set on rising/falling edge. */
   MOD_ON_MSK |= _BV(MOD1_ON_INT) | _BV(MOD2_ON_INT);

   /* Initialie SPI. */
   spim_init();

   /* Detect card 1. */
   mod_data[0].on = MOD1_ON_PIN & _BV(MOD1_ON_P);

   /* Detect card 2. */
   mod_data[1].on = MOD2_ON_PIN & _BV(MOD2_ON_P);
}


