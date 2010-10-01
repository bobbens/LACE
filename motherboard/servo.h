


#ifndef _SERVO_H
#  define _SERVO_H


#include <stdint.h>


/**
 * @note Uses TIMER1.
 */
void servo_init1 (void);
void servo_pwm1A( int16_t pwm );
void servo_pwm1B( int16_t pwm );


#endif /* _SERVO_H */
