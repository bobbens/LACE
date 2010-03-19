
#include "global.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include <stdint.h>

#include "ioconf.h"
#include "comm.h"
#include "uart.h"
#include "adc.h"
#include "motors.h"
#include "encoder.h"
#include "spis.h"
#include "sched.h"


/*
 * Scheduler defines.
 */
/*
 * Scheduler divider.  Formula is:
 *
 *  f_task = f_sched / DIVIDER
 *
 * Example
 *  10 Hz  = 20 kHz / 2000
 *  100 Hz = 20 kHz / 200
 *  1 kHz  = 20 kHz / 20
 *  10 kHz = 20 kHz / 2
 */
static volatile uint16_t sched_counter = 0; /**< Scheduler counter. */
#define SCHED_MOTOR_DIVIDER         60  /**< Motor control divider. */
#define SCHED_HEARTBEAT_DIVIDER     200 /**< Divider for heartbeat. */
#define SCHED_MAX_DIVIDER           600 /**< Overflow amount for scheduler divider. */
/* Scheduler state flags. */
volatile unsigned int sched_flags  = 0; /**< Scheduler flags. */


/*
 * Prototypes.
 */
/* Scheduler. */
static void sched_init (void);
static void sched_run( uint8_t flags );
/* Heartbeat. */
static void heartbeat_init (void);
static void heartbeat_set( uint16_t rate );
static void heartbeat_update (void);


/*
 *
 *    H E A R T B E A T
 *
 */
static uint16_t heartbeat_counter = 0; /**< Heartbeat counter. */
static uint16_t heartbeat_target  = 0; /**< Heartbeat target. */
/**
 * @brief Initializes the heartbeat.
 */
static void heartbeat_init (void)
{
   heartbeat_counter = 0;
   LED0_ON();
   LED1_OFF();
   heartbeat_set( 50 );
}
/**
 * @brief Sets the heartbeat rate, should be in 1/100 seconds.
 *
 *    @param rate Rate to set.
 */
static void heartbeat_set( uint16_t rate )
{
   heartbeat_target = rate;
}
/**
 * @brief Toggles the heartbeat if needed.
 */
static void heartbeat_update (void)
{
   heartbeat_counter++;
   if (heartbeat_counter >= heartbeat_target) {
      LED0_TOG();
      LED1_TOG();
      heartbeat_counter = 0;
   }

   /* Reset watchdog. */
   wdt_reset();
}


/*
 *
 *   S C H E D U L E R
 *
 */
/**
 * @brief Scheduler interrupt on timer1 overflow.
 *
 * @note Running at 20 kHz.
 */
ISR( TIMER1_OVF_vect )
{
   /* Increment Encoder 0 timer. */
   if (enc0.cur_tick < UINT16_MAX) /* Avoid overflow. */
      enc0.cur_tick++;
   else /* Overflow. */
      enc0.last_tick = enc0.cur_tick;

   /* Increment Encoder 1 timer. */
   if (enc1.cur_tick < UINT16_MAX) /* Avoid overflow. */
      enc1.cur_tick++;
   else /* Overflow. */
      enc1.last_tick = enc1.cur_tick;

   /* Do some scheduler stuff here. */
   if (!(sched_counter % SCHED_HEARTBEAT_DIVIDER))
      sched_flags |= SCHED_HEARTBEAT;
   if (!(sched_counter % SCHED_MOTOR_DIVIDER))
      sched_flags |= SCHED_MOTOR;
   sched_counter = (sched_counter+1) % SCHED_MAX_DIVIDER;
}
/**
 * @brief Initializes the scheduler on Timer1.
 */
static void sched_init (void)
{
   /* Phase and freq correct mode.
    *
    * f_pwm = f_clk / (2 * N * TOP)
    *
    *  1 kHz = 20 MHz / (2 * 8 * 1250)
    * 20 kHz = 20 MHz / (2 * 1 * 500)
    * 50 kHz = 20 Mhz / (2 * 1 * 200)
    */
   TCCR1A = _BV(WGM10) | /* Phase and freq correct mode with OCR1A. */
         0; /* No actual PWM output. */
   TCCR1B = _BV(WGM13) | /* Phase and freq correct mode with OCR1A. */
         _BV(CS10); /* 1 prescaler. */
#if 0
         _BV(CS11); /* 8 prescaler */
         _BV(CS11) | _BV(CS10); /* 64 prescaler */
         _BV(CS12); /* 256 prescaler */
         _BV(CS12) | _BV(CS10); /* 1024 prescaler */
#endif
   TIMSK1 = _BV(TOIE1); /* Enable Timer1 overflow. */
   OCR1A  = 500;

   /* Initialize flags. */
   sched_flags = 0;
}


/**
 * @brief Runs the scheduler.
 *
 *    @param flags Current scheduler flags to use.
 */
static void sched_run( uint8_t flags )
{
   /*
    * Run tasks.
    *
    * Very important that the periodicity of the scheduler update task
    *  allows this to finish or the loss of task execution may occur.
    */
   if (flags & SCHED_HEARTBEAT) {
      heartbeat_update();
   }
   if (flags & SCHED_MOTOR) {
      motor_control();
   }
}


/**
 * @brief Initializes all the subsystems.
 */
static void init (void)
{
   int reset_source;

   /* Enable LED. */
   LED0_INIT();
   LED1_INIT();

   /* Heartbeat init. */
   heartbeat_init();

   /* Communication subsystem. */
   spis_init();

   /* Motor subsystem. */
   motor_init();
   encoder_init();

   /* Power management. */
   PRR = _BV(PRTWI) | /* Disable TWI. */
         _BV(PRTIM2) | /* Disable Timer 2. */
         _BV(PRADC); /* Disable ADC. */

   /* Sensors init. */
#if 0
   adc_init();
   DDRA &= ~_BV(PORTA4); /* All ADC are inputs. */
   /*sensors_init();*/
#endif

   /* Initialize communication. */
   comm_init();

   /* Initialize the scheduler. */
   sched_init();

   /* Enable interrupts. */
   sei();

   printf( "Motor control board online...\n" );
   /* Check why we reset. */
   reset_source = MCUSR; 
   MCUSR = _BV(WDRF) | _BV(BORF) | _BV(EXTRF) | _BV(PORF); /* Clear flags. */
   if (reset_source & _BV(PORF))
      printf("Power-on Reset\n"); 
   else if (reset_source & _BV(EXTRF))
      printf("External Reset\n");
   else if (reset_source & _BV(BORF))
      printf("Brownout Reset\n");
   else if (reset_source & _BV(WDRF))
      printf("Watchdog Reset\n");
}


/**
 * @brief Entry point.
 */
int main (void)
{
#if 0
   int i;
   LED0_INIT();
   LED1_INIT();
   LED0_ON();
   LED1_OFF();
   MOTOR0_DDR1 |= _BV(MOTOR0_IN1);
   MOTOR0_DDR2 |= _BV(MOTOR0_IN2);
   MOTOR1_DDR1 |= _BV(MOTOR1_IN1);
   MOTOR1_DDR2 |= _BV(MOTOR1_IN2);
   TCCR0A = _BV(COM0A1) | _BV(COM0B1) |
            _BV(WGM00) | _BV(WGM01); /* Fast PWM mode. */
   TCCR0B = _BV(CS01); /* 8 prescaler. */
   i        = 0;
   OCR0A    = i;
   OCR0B    = i;
   while (1) {
      _delay_ms(500.);
      LED0_TOG();
      LED1_TOG();
      i       += 10;
      OCR0A    = i;
      OCR0B    = i;
   }
#endif
#if 0
   LED0_INIT();
   LED1_INIT();
   LED0_ON();
   LED1_OFF();
   /*uart_init( UART_BAUD_SELECT(9600, F_CPU ) );*/
   /*rs232_init0( USART_4_8k );*/
   comm_init();
   sei();
   while (1) {
      printf("Hello World!\n");
      _delay_ms(1000.);
      LED0_TOG();
      LED1_TOG();
   }
#endif
#if 0
   LED0_INIT();
   LED1_INIT();
   LED0_ON();
   LED1_OFF();
   spis_init();
   sei();
   while (1) {
      _delay_ms( 500. );
      LED0_TOG();
   }
#endif
   uint8_t flags;

   /* Disable watchdog timer since it doesn't always get reset on restart. */
   wdt_disable();

   /* Set sleep mode. */
   set_sleep_mode( SLEEP_MODE_IDLE );

   /* Initialize subsystems. */
   init();

   /* Set up watchdog timer. */
   wdt_reset(); /* Just in case. */
   wdt_enable(WDTO_15MS);

   /* Main loop. */
   while (1) {
      /* Atomic test to see if has anything to do. */
      cli();
      if (sched_flags != 0) {

         /* Atomic store temporary flags and reset real flags in case we run a bit late. */
         flags = sched_flags;
         sched_flags = 0;
         sei(); /* Restart interrupts. */

         /* Run scheduler. */
         sched_run( flags );
      }
      /* Sleep. */
      else {
         /* Atomic sleep as specified on the documentation. */
         sleep_enable();
         sei();
         sleep_cpu();
         sleep_disable();
      }
   }
}

