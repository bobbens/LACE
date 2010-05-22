

#ifndef _MOD_HBRIDGE_H
#  define _MOD_HBRIDGE_H


#include <stdint.h>


/**
 * @brief Attempts to detect and initialize the existance of the Dual H-Bridge module on a port.
 *
 *    @param port Port to detect the Dual H-Bridge module on.
 *    @return 0 if module was detected and initialized properly, -1 if it wasn't.
 */
int dhb_init( int port );


/**
 * @brief Sets the motor controller mode.
 *
 *    @param port Port the module is on.
 *    @param mode Mode to set.
 *    @return 0 on success.
 */
int dhb_mode( int port, char mode );


/**
 * @brief Sets the targets of each motor, these are dependent on the operating mode.
 *
 * >0 is forward
 * =0 is brake
 * <0 is backwards
 *
 *    @param port Port the dhb board is on.
 *    @param t0 Target for motor 0.
 *    @param t1 Target for motor 1.
 *    @return 0 on success.
 */
int dhb_target( int port, int16_t t0, int16_t t1 );


/**
 * @brief Gets the feedback of the motors, these are dependent on the operating mode.
 *
 *    @param[out] m0 Same units as the target for motor 0 feedback.
 *    @param[out] m1 Same units as the target for motor 1 feedback.
 *    @return 0 on success.
 */
int dhb_feedback( uint16_t *m0, uint16_t *m1 );


#endif /* _MOD_HBRIDGE_H */


