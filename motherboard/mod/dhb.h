

#ifndef _MOD_HBRIDGE_H
#  define _MOD_HBRIDGE_H


#include <stdint.h>

#include "dhb/hbridge.h"


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
 *    @return 0 on success.
 */
int dhb_feedback( int port );
void dhb_feedbackValue( int port, int16_t *mota, int16_t *motb );


/**
 * @brief Gets the feedback of the motors, these are dependent on the operating mode.
 *
 *    @return 0 on success.
 */
int dhb_current( int port );
void dhb_currentValue( int port, uint16_t *mota, uint16_t *motb );


#endif /* _MOD_HBRIDGE_H */


