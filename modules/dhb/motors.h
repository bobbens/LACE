
#ifndef _MOTORS_H
#  define _MOTORS_H


#include <stdint.h>


#define MOTOR_CONTROL_HZ      1220 /**< Motor control frequency. */


/**
 * @brief Motor control structure.
 */
typedef struct motor_s {
   /* Last tick. */
   int16_t feedback;

   /* Target. */
   uint16_t target; /**< Target velocity. */

   /* Internal usage variables. */
   int16_t e_accum; /**< Accumulated error, for integral part. */

   /* Controller parameters - these are divided by 16 (>>4). */
   uint8_t kp; /**< Proportional part of the controller. */
   uint8_t ki; /**< Integral part of the controller. */
   int16_t windup; /**< Windup limit. */
} motor_t;


extern motor_t mot0; /**< Motor 0. */
extern motor_t mot1; /**< Motor 1. */


inline void motor_init (void);
inline void motor_control (void);
inline void motor_mode( int mode );
inline void motor_set( int16_t motor_0, int16_t motor_1 );


#endif /* _MOTORS_H */
