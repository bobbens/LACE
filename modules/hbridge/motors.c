
#include "motors.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "ioconf.h"
#include "encoder.h"


/*
 * Motor Truth table:
 *
 *    IN1   IN2    OUT1   OUT2    DESC
 *     H     H      L      L      Brake
 *     H     L      H      L      Forward
 *     L     H      L      H      Backwards
 *     L     L      Z      Z      Release
 */
/* Motor 0. */
#define MOTOR0_FORWARD() \
   MOTOR0_PORT1 |=  _BV(MOTOR0_IN1); \
   MOTOR0_PORT2 &= ~_BV(MOTOR0_IN2)
#define MOTOR0_BACKWARDS() \
   MOTOR0_PORT1 &= ~_BV(MOTOR0_IN1); \
   MOTOR0_PORT2 |=  _BV(MOTOR0_IN2)
#define MOTOR0_BRAKE() \
   MOTOR0_PORT1 |=  _BV(MOTOR0_IN1); \
   MOTOR0_PORT2 |=  _BV(MOTOR0_IN2)
#define MOTOR0_RELEASE() \
   MOTOR0_PORT1 &= ~_BV(MOTOR0_IN1); \
   MOTOR0_PORT2 &= ~_BV(MOTOR0_IN2)
/* Motor 1. */
#define MOTOR1_FORWARD() \
   MOTOR1_PORT1 |=  _BV(MOTOR1_IN1); \
   MOTOR1_PORT2 &= ~_BV(MOTOR1_IN2)
#define MOTOR1_BACKWARDS() \
   MOTOR1_PORT1 &= ~_BV(MOTOR1_IN1); \
   MOTOR1_PORT2 |=  _BV(MOTOR1_IN2)
#define MOTOR1_BRAKE() \
   MOTOR1_PORT1 |=  _BV(MOTOR1_IN1); \
   MOTOR1_PORT2 |=  _BV(MOTOR1_IN2)
#define MOTOR1_RELEASE() \
   MOTOR1_PORT1 &= ~_BV(MOTOR1_IN1); \
   MOTOR1_PORT2 &= ~_BV(MOTOR1_IN2)


/**
 * @brief Motor control structure.
 */
typedef struct motor_s {
   /* Target. */
   int8_t dir; /**< Target direction. */
   uint16_t target; /**< Target velocity. */

   /* Internal usage variables. */
   int16_t e_accum; /**< Accumulated error, for integral part. */

   /* Controller parameters - these are divided by 16 (>>4). */
   uint8_t kp; /**< Proportional part of the controller. */
   uint8_t ki; /**< Integral part of the controller. */
   int16_t windup; /**< Windup limit. */
} motor_t;


/*
 * The motors.
 */
static motor_t mot0;
static motor_t mot1;


/*
 * Prototypes.
 */
static void _motor_init( motor_t *mot );
static uint8_t _motor_control( motor_t *mot, encoder_t *enc );


/**
 * @brief Clears a motor.
 *
 *    @param mot Motor to clear.
 */
static void _motor_init( motor_t *mot )
{
   /* Target to seek out. */
   mot->target  = 0;

   /* Internal use variables. */
   mot->e_accum = 0;

   /* Controller parameters. */
   mot->kp      = 100;
   mot->ki      = 5;
   mot->windup  = 816;
}



/**
 * @brief Initializes the motors.
 */
void motor_init (void)
{
   /* Enable motor 0. */
   MOTOR0_DDR1 |= _BV(MOTOR0_IN1);
   MOTOR0_DDR2 |= _BV(MOTOR0_IN2);

   /* Enable motor 1. */
   MOTOR1_DDR1 |= _BV(MOTOR1_IN1);
   MOTOR1_DDR2 |= _BV(MOTOR1_IN2);

   /* Initialize pwm.
    *
    * We'll want the fast PWM mode wih the 64 prescaler.
    *
    *    f_pwm = f_clk / (256 * N)
    *    f_pwm = 20 MHz / (256 * 8)  = 9.76 kHz
    *    f_pwm = 20 MHz / (256 * 64) = 1.22 kHz
    */
   TCCR0A = _BV(WGM00) | _BV(WGM01); /* Fast PWM mode. */
   /*TCCR0B = _BV(CS01)  | _BV(CS00);*/ /* 64 prescaler */
   TCCR0B = _BV(CS01); /* 8 prescaler. */
   /* Start both motors stopped. */
   OCR0A  = 0;
   OCR0B  = 0;

   /* Clear targets. */
   _motor_init( &mot0 );
   _motor_init( &mot1 );

   /* Start out with motors off. */
   motor_set( 0, 0 );
}


/**
 * @brief Control routine for a motor.
 *
 * This routine will update the motor based on the current control model.
 *
 *         +-----------+      Ts /              +-----------+
 *  +      |  Ts * Ki  | +      /     +-----+   |     K     |
 * --(+)-->| --------- |--(+)--/   -->| Zoh |-->| --------- |---+-->
 *   -|    |   z - 1   |  +|          +-----+   | t * s + 1 |   |
 *    |    +-----------+   |                    +-----------+   |
 *    |                    |                                    |
 *    |                 +------+                                | 
 *    |                 |  Kp  |                                |
 *    |                 +------+                     \ Ts       |
 *    |                    |             +-----+      \         |
 *    +--------------------+-------------|  n  |----   \--------+
 *                                       +-----+
 *
 * @note Using 16 bit numbers for calculations using 8 bits for the significant
 *       numbers and 8 bits for the "decimals".
 */
static uint8_t _motor_control( motor_t *mot, encoder_t *enc )
{
   int16_t feedback, error, output;
   uint8_t pwm;

   /* Only matters if we have a target. */
   if (mot->target == 0) {
      return 0;
   }

   /* Linearization of the feedback.
    *
    *    ticks[20kHz]   N encoder turn        1 second
    * X  ------------ * -------------- * -------------------   ===>
    *    encoder turn    1 revolution     20000 ticks[50kHz]
    *
    *
    *  20000 / N
    *  ---------  = revolutions per second
    *      X
    */
   feedback = 5000 / enc->last_tick;

   /* Calculate the error. */
   error    = mot->target - feedback;

   /* Accumulate error. */
   mot->e_accum += error;
   /* Anti-windup. */
   if (mot->e_accum > mot->windup)
      mot->e_accum = mot->windup;
   else if (mot->e_accum < -mot->windup)
      mot->e_accum = -mot->windup;

   /* Run control - PI. */
   output   = (error * mot->kp) >> 4; /* P */
   output  += (mot->e_accum * mot->ki) >> 4; /* I */

   /* Get PWM output, note we can't do backwards. */
   if (output > 255)
      pwm = 0xFF;
   else if (output < 128)
      pwm = 128;
   else
      pwm = output;

   /* It's inverted. */
   return pwm;
}


/**
 * @brief Runs the control routine on both motors.
 */
__inline void motor_control (void)
{
   /* Control loop. */
   OCR0A = _motor_control( &mot0, &enc0 );
   OCR0B = _motor_control( &mot1, &enc1 );
}


/**
 * @brief Sets the motor velocity.
 */
void motor_set( int16_t motor_0, int16_t motor_1 )
{
   /* Motor 0. */
   mot0.target  = motor_0;
   mot0.e_accum = 0; /* Clear accumulator just in case. */
   if (motor_0 == 0) {
      TCCR0A &= ~(_BV(COM0A1) | _BV(COM0A0)); /* Disable PWM output. */
      MOTOR0_BRAKE();
   }
   else {
      TCCR0A |= _BV(COM0A0); /* Activate PWM output again. */
      if (motor_0 > 0) {
         MOTOR0_PORT2 &= ~_BV(MOTOR0_IN2); /* Forward mode. */
      }
      else if (motor_0 < 0) {
         MOTOR0_PORT2 |=  _BV(MOTOR0_IN2); /* Backwards mode. */
      }
   }
   /* Motor 1. */
   mot1.target  = motor_1;
   mot1.e_accum = 0; /* Clear accumulator just in case. */
   if (motor_1 == 0) {
      TCCR1A &= ~(_BV(COM1A1) | _BV(COM1A1)); /* Disable PWM output. */
      MOTOR1_BRAKE();
   }
   else {
      TCCR1A |= _BV(COM1A1); /* Activate PWM output again. */
      if (motor_1 > 0) {
         MOTOR1_PORT2 &= ~_BV(MOTOR1_IN2); /* Forward mode. */
      }
      else if (motor_1 < 0) {
         MOTOR1_PORT2 |=  _BV(MOTOR1_IN2); /* Backwards mode. */
      }
   }
}


/**
 * @brief Gets the motor velocity.
 */
void motor_get( int16_t *motor_0, int16_t *motor_1 )
{
   *motor_0 = enc0.last_tick;
   *motor_1 = enc1.last_tick;
}
