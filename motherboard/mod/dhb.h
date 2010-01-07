

#ifndef _MOD_HBRIDGE_H
#  define _MOD_HBRIDGE_H


#include <stdint.h>


/**
 * @brief Operating modes of the Dual H-Bridge module.
 */
typedef enum dhb_mode_e {
   DHB_MODE_OPEN, /**< Open loop mode - target in PWM width. */
   DHB_MODE_ENC_SIMPLE, /**< Simple encoder mode - target in RPM. */
   DHB_MODE_ENC_QUAD, /**< Quadrature encoder mode - Unsupported. */
   DHB_MODE_TORQUE, /**< Torque control mode - Unsupported. */
} dhb_mode_t;


/**
 * @brief Attempts to detect and initialize the existance of the Dual H-Bridge module on a port.
 *
 *    @param port Port to detect the Dual H-Bridge module on.
 *    @return 0 if module was detected and initialized properly, -1 if it wasn't.
 */
int dhb_init( int port );


/**
 * @brief Sets the operating mode of the Dual H-Bridge module.
 */
void dhb_mode( dhb_mode_t mode );


/**
 * @brief Sets the targets of each motor, these are dependent on the operating mode.
 *
 * >0 is forward
 * =0 is brake
 * <0 is backwards
 *
 *    @param t0 Target for motor 0.
 *    @param t1 Target for motor 1.
 */
void dhb_target( uint16_t t0, uint16_t t1 );


/**
 * @brief Gets the feedback of the motors, these are dependent on the operating mode.
 *
 *    @param[out] m0 Same units as the target for motor 0 feedback.
 *    @param[out] m1 Same units as the target for motor 1 feedback.
 */
void dhb_feedback( uint16_t *m0, uint16_t *m1 );


#endif /* _MOD_HBRIDGE_H */
