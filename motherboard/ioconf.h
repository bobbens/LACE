


#ifndef _IOCONF_H
#  define _IOCONF_H


#include <avr/io.h>


/* CPU frequency. */
#define F_CPU              20000000UL


/* PWM. */
#define PWM0_DDR           DDRB
#define PWM0_PORT          PORTB
#define PWM0A              PB3
#define PWM0B              PB4
#define PWM1_DDR           DDRD
#define PWM1_PORT          PORTD
#define PWM1A              PD5
#define PWM1B              PD4
#define PWM2_DDR           DDRD
#define PWM2_PORT          PORTD
#define PWM2A              PD7
#define PWM2B              PD6


/* LED. */
#define LED0_DDR           DDRB
#define LED0_PORT          PORTB
#define LED0               PB2
#define LED0_INIT()        LED0_DDR |= _BV(LED0)
#define LED0_ON()          LED0_PORT &= ~_BV(LED0)
#define LED0_OFF()         LED0_PORT |=  _BV(LED0)
#define LED0_TOGGLE()      LED0_PORT ^= _BV(LED0)
#define LED0_STATUS()      (LED0_PORT & _BV(LED0))


/* SPI. */
#define SPI_DDR            DDRB
#define SPI_SS             PB4
#define SPI_MOSI           PB5
#define SPI_MISO           PB6
#define SPI_SCK            PB7


/* Module Global. */
#define MOD_ON_INT         PCIE2
#define MOD_ON_MSK         PCMSK2
#define MOD_ON_SIG         PCINT2_vect
/* Module 1. */
#define MOD1_RST_DDR       DDRC
#define MOD1_RST_PORT      PORTC
#define MOD1_RST_P         PC6
#define MOD1_ON_DDR        DDRC
#define MOD1_ON_PORT       PORTC
#define MOD1_ON_PIN        PINC
#define MOD1_ON_P          PC4
#define MOD1_ON_INT        PCINT20
#define MOD1_SS_DDR        DDRB
#define MOD1_SS_PORT       PORTB
#define MOD1_SS_P          PB0
/* Module 2. */
#define MOD2_RST_DDR       DDRC
#define MOD2_RST_PORT      PORTC
#define MOD2_RST_P         PC7
#define MOD2_ON_DDR        DDRC
#define MOD2_ON_PORT       PORTC
#define MOD2_ON_PIN        PINC
#define MOD2_ON_P          PC5
#define MOD2_ON_INT        PCINT20
#define MOD2_SS_DDR        DDRB
#define MOD2_SS_PORT       PORTB
#define MOD2_SS_P          PB1


#endif /* _IOCONF_H */


