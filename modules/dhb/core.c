
#include "global.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <util/delay.h>

#include <stdio.h>
#include <stdint.h>

#include "ioconf.h"
#include "comm.h"
#include "uart.h"
#include "motors.h"
#include "encoder.h"
#include "spis.h"
#include "sched.h"
#include "current.h"


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
 *  33 Hz  = 20 kHz / 600
 *  100 Hz = 20 kHz / 200
 *  333 Hz = 20 kHz / 60
 *  1 kHz  = 20 kHz / 20
 *  10 kHz = 20 kHz / 2
 */
static uint8_t sched_mot_counter = 0; /**< Counter for the motor controller. */
#define SCHED_MOTOR_TOP       60  /**< Motor control divider. */
static uint8_t sched_heartbeat_counter = 0; /**< Counter for the heart beat. */
#define SCHED_HEARTBEAT_TOP  200 /**< Divider for heartbeat. */
/* Scheduler state flags. */
volatile uint8_t sched_flags  = 0; /**< Scheduler flags. */


/*
 * Prototypes.
 */
/* Scheduler. */
static inline void sched_init (void);
static inline void sched_run( uint8_t flags );
/* Heartbeat. */
static inline void heartbeat_init (void);
static inline void heartbeat_set( uint16_t rate );
static inline void heartbeat_update (void);


/*
 *
 *    H E A R T B E A T
 *
 */
static uint8_t heartbeat_counter = 0; /**< Heartbeat counter. */
static uint8_t heartbeat_target  = 0; /**< Heartbeat target. */
/**
 * @brief Initializes the heartbeat.
 */
static inline void heartbeat_init (void)
{
   heartbeat_counter = 0;
   LED0_ON();
   LED1_OFF();
   heartbeat_set( 50 );
}
/**
 * @brief Sets the heartbeat rate, should be in 1/10 seconds.
 *
 *    @param rate Rate to set.
 */
static inline void heartbeat_set( uint16_t rate )
{
   heartbeat_target = rate;
}
/**
 * @brief Toggles the heartbeat if needed.
 */
static inline void heartbeat_update (void)
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
   sched_mot_counter++;
   if (sched_mot_counter >= SCHED_MOTOR_TOP) {
      sched_flags |= SCHED_MOTOR;
      sched_mot_counter = 0;
   }
   sched_heartbeat_counter++;
   if (sched_heartbeat_counter >= SCHED_HEARTBEAT_TOP) {
      sched_flags |= SCHED_HEARTBEAT;
      sched_heartbeat_counter = 0;
   }
}
/**
 * @brief Initializes the scheduler on Timer1.
 */
static inline void sched_init (void)
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
static inline void sched_run( uint8_t flags )
{
   uint8_t i;
   /*
    * Run tasks.
    *
    * Very important that the periodicity of the scheduler update task
    *  allows this to finish or the loss of task execution may occur.
    */
   if (flags & SCHED_HEARTBEAT) {
      heartbeat_update();
      current_startSample();
   }
   if (flags & SCHED_SPIS_PREP_MOTORGET) {
      /*
      spis_buf[0] = (uint8_t)(mot0.feedback>>8);
      spis_buf[1] = (uint8_t)mot0.feedback;
      spis_buf[2] = (uint8_t)(mot1.feedback>>8);
      spis_buf[3] = (uint8_t)mot1.feedback;
       */
      spis_buf[0] = 0x31;
      spis_buf[1] = 0x32;
      spis_buf[2] = 0x33;
      spis_buf[3] = 0x34;
      for (i=0; i<4; i++)
         spis_crc    = _crc_ibutton_update( spis_crc, spis_buf[i] );
   }
   if (flags & SCHED_SPIS_PREP_CURRENT) {
      /*
      spis_buf[0] = current_buffer[0];
      spis_buf[1] = current_buffer[1];
      spis_buf[2] = current_buffer[2];
      spis_buf[3] = current_buffer[3];
       */
      spis_buf[0] = 0x41;
      spis_buf[1] = 0x42;
      spis_buf[2] = 0x43;
      spis_buf[3] = 0x44;
      for (i=0; i<4; i++)
         spis_crc    = _crc_ibutton_update( spis_crc, spis_buf[i] );
   }
   if (flags & SCHED_MOTOR) {
      motor_control();
   }
}


/**
 * @brief Initializes all the subsystems.
 */
static inline void init (void)
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

   /* ADC subsystem. */
   current_init();

   /* Power management. */
   PRR = _BV(PRTWI) | /* Disable TWI. */
         _BV(PRTIM2) | /* Disable Timer 2. */
         _BV(PRUSART0); /* Disable USART0. */

   /* Initialize communication. */
#ifdef DEBUG
   comm_init();
#endif /* DEBUG */

   /* Initialize the scheduler. */
   sched_init();

   /* Enable interrupts. */
   sei();

   dprintf( "DHB online... " );
   /* Check why we reset. */
   reset_source = MCUSR; 
   MCUSR = _BV(WDRF) | _BV(BORF) | _BV(EXTRF) | _BV(PORF); /* Clear flags. */
   if (reset_source & _BV(PORF))
      dprintf("(PWR Rst)\n"); 
   else if (reset_source & _BV(EXTRF))
      dprintf("(EXT Rst)\n");
   else if (reset_source & _BV(BORF))
      dprintf("(BDO Rst)\n");
   else if (reset_source & _BV(WDRF))
      dprintf("(WDT Rst)\n");
}


/**
 * @brief Entry point.
 */
int main (void)
{
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
         flags       = sched_flags;
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

