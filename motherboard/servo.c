

#include "servo.h"

#include <avr/interrupt.h>


#define SERVO1_MIN   2500
#define SERVO1_MAX   5000


void servo_init1 (void)
{
   /* Initialize pwm.
    *
    * Timer fast pwm.
    *
    *            f_core            20 MHz
    * f_pwm = -------------- = --------------- = 50 Hz = 20 ms
    *          N * (1 + TOP)   8 * (1 + 49999)
    *
    * 1 ms = 2500
    * 2 ms = 5000
    *
    *  COM1A1    COM1B1
    *    1         0       non-inverting mode
    *
    *
    *  WGM13  WGM12  WGM11  WGM10 | Operation  TOP   Update  TOV1 Flag
    *    1      1      1      0   |  Fast PWM  ICR1  BOTTOM     TOP
    *
    *
    *  CS12  CS11  CS10
    *   0     1     0     clk_io / 8 (from prescaler)
    *
    *
    *  TCCR1A -> COM1A1, COM1A0, COM1B1, COM1B0, WGM11, WGM10
    *
    *  TCCR1B -> WGM13, WGM12, CS12, CS11, CS10
    *
    *  ICR1 = 49999
    */
   TCCR1A = /*_BV(WGM10) |*/ _BV(WGM11) | /* Fast PWM mode. */
            _BV(COM1A1) /*| _BV(COM1B1)*/; /* Non-inverting mode. */
   /*TCCR0B = _BV(CS01)  | _BV(CS00);*/ /* 64 prescaler */
   TCCR1B = _BV(WGM12) | _BV(WGM13) | /* Faste PWM. */
            _BV(CS11); /* 8 prescaler. */
   TIMSK1 = 0; /* No interrupts. */
   /* Set freq. */
   ICR1   = 49999;
   /* Start both motors stopped. */
   OCR1A  = (SERVO1_MIN + SERVO1_MAX) / 2;
}


void servo_pwm1A( int16_t pwm )
{
   OCR1A = pwm;
}


void servo_pwm1B( int16_t pwm )
{
   OCR1B = pwm;
}


