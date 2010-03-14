

#ifndef PINOUT_H
#  define PINOUT_H


#include <avr/io.h>

/*
 * LEDs
 */
#define LED0_PORT    PORTB
#define LED0_DDR     DDRB
#define LED0_PIN     PB0
#define LED1_PORT    PORTB
#define LED1_DDR     DDRB
#define LED1_PIN     PB1
#define LED0_INIT()  LED0_DDR  |= _BV(LED0_PIN)
#define LED0_OFF()   LED0_PORT |= _BV(LED0_PIN)
#define LED0_ON()    LED0_PORT &= ~_BV(LED0_PIN)
#define LED0_TOG()   LED0_PORT ^= _BV(LED0_PIN)
#define LED1_INIT()  LED1_DDR  |= _BV(LED1_PIN)
#define LED1_OFF()   LED1_PORT |= _BV(LED1_PIN)
#define LED1_ON()    LED1_PORT &= ~_BV(LED1_PIN)
#define LED1_TOG()   LED1_PORT ^= _BV(LED1_PIN)

/*
 * Motors.
 */
/* Motor 0. */
#  define MOTOR0_DDR1        DDRD
#  define MOTOR0_PORT1       PORTD
#  define MOTOR0_IN1         PD6
#  define MOTOR0_DDR2        DDRC
#  define MOTOR0_PORT2       PORTC
#  define MOTOR0_IN2         PC4
/* Motor 1. */
#  define MOTOR1_DDR1        DDRD
#  define MOTOR1_PORT1       PORTD
#  define MOTOR1_IN1         PD5
#  define MOTOR1_DDR2        DDRC
#  define MOTOR1_PORT2       PORTC
#  define MOTOR1_IN2         PC5


/*
 * Encoders.
 */
#  define ENCODER_INT        PCIE1    /**< Global interrupt mask for encoders. */
#  define ENCODER_MSK        PCMSK1   /**< Interrupt mask register for encoders. */
#  define ENCODER_INT0       PCINT8   /**< Interrupt pin encoder A is on. */
#  define ENCODER_INT1       PCINT10  /**< Interrupt pin encoder B is on. */
#  define ENCODER_VECT       PCINT1_vect /**< Interrupt vector for encoders. */

#  define ENCODER_DDR        DDRC     /**< DDR register for encoders. */
#  define ENCODER_PIN        PINC     /**< Pin encoders are on. */
#  define ENCODER_PORT0      PC0      /**< Encoder A port. */
#  define ENCODER_PORT1      PC2      /**< Encoder B port. */
#  define ENCODER_PIN0       ENCODER_PORT0 /**< Encoder A pin. */
#  define ENCODER_PIN1       ENCODER_PORT1 /**< Encoder B pin. */


#endif /* PINOUT_H */


