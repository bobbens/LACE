
#ifndef _MOTORS_H
#  define _MOTORS_H


#include <stdint.h>


#define MOTOR_CONTROL_HZ      1220 /**< Motor control frequency. */


void motor_init (void);
void motor_control (void);
void motor_set( int16_t motor_0, int16_t motor_1 );
void motor_get( int16_t *motor_0, int16_t *motor_1 );


#endif /* _MOTORS_H */
