
#ifndef _MOTORS_H
#  define _MOTORS_H


#include <stdint.h>


#define MOTOR_CONTROL_HZ      1220 /**< Motor control frequency. */


void motor_init (void);
void motor_control (void);
void motor_mode( int mode );
void motor_set( int16_t motor_0, int16_t motor_1 );
void motor_get( uint8_t *out );


#endif /* _MOTORS_H */
