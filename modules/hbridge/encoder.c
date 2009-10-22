

#include "encoder.h"

#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "ioconf.h"


encoder_t enc0; /**< Encoder 1. */
encoder_t enc1; /**< Encoder 2. */


/*
 * Prototypes.
 */
static void _encoder_init( encoder_t *enc, uint8_t pinstate );


/**
 * @brief Encoder interrupt.
 */
ISR( ENCODER_VECT )
{
   uint8_t inp;

   /* Check to see if encoder 0 changed. */
   inp = ENCODER_PIN & _BV(ENCODER_PIN0);
   if (inp != enc0.pin_state) { /* See if state changed. */
      enc0.pin_state = inp;
      enc0.last_tick = enc0.cur_tick; /* Last tick is current tick. */
      enc0.cur_tick  = 0; /* Reset counter. */
   }

   /* Check to see if encoder 1 changed. */
   inp = ENCODER_PIN & _BV(ENCODER_PIN1);
   if (inp != enc1.pin_state) { /* See if state changed. */
      enc1.pin_state = inp;
      enc1.last_tick = enc1.cur_tick; /* Last tick is current tick. */
      enc1.cur_tick  = 0; /* Reset counter. */
   }

}


/**
 * @brief Initializes a single encoder.
 *
 *    @param enc Encoder to initialize.
 *    @param pinstate Current pinstate.
 */
static void _encoder_init( encoder_t *enc, uint8_t pinstate )
{
   enc->cur_tick  = 0;
   enc->last_tick = UINT16_MAX; /* Consider stopped. */
   enc->pin_state = pinstate;
}



/**
 * @brief Sets up the encoders.
 */
void encoder_init (void)
{
   /* Set pins as input. */
   ENCODER_DDR &= ~(_BV(ENCODER_PORT0) | _BV(ENCODER_PORT1));

   /* Initialize encoders. */
   _encoder_init( &enc0, (ENCODER_PIN & _BV(ENCODER_PIN0)) );
   _encoder_init( &enc1, (ENCODER_PIN & _BV(ENCODER_PIN1)) );

   /* Set up interrupts. */
   PCICR       |= _BV(ENCODER_INT);
#if 0
   GIMSK       |= _BV(ENCODER_INT);
#endif
   ENCODER_MSK |= _BV(ENCODER_INT1) | _BV(ENCODER_INT0); /* Enabled encoder interrupts. */
   MCUCR       |= /*_BV(ISC01) |*/ _BV(ISC00); /* Set on rise/falling edge. */
}


