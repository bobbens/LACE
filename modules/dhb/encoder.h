

#ifndef ENCODERS_H
#  define ENCODERS_H


#include <stdint.h>


/**
 * @brief The encoder structure.
 */
typedef struct encoder_s {
   uint16_t cur_tick; /**< Current tick (counts up with overflow). */
   uint16_t last_tick; /**< Last tick to register a state change. */
   uint8_t  pin_state; /**< Current pin state. */
} encoder_t;


/*
 * Current encoder values.
 *
 * We assume that we're going fast enough to not overflow 8 bits.
 */
extern encoder_t enc0; /**< Encoder 0 counter. */
extern encoder_t enc1; /**< Encoder 1 counter. */


/*
 * Initialization.
 */
inline void encoder_init (void);


#endif /* ENCODERS_H */


