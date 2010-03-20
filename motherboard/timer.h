

#ifndef _TIMER_H
#  define _TIMER_H


/**
 * @file
 *
 * @brief Timer infrastructure for the LACE motherboard.
 *
 * @note This uses TIMER0.
 */


#define MAX_TIMERS   3 /**< Maximum number of available timers. */


/**
 * @brief Initializes the timer infrastructure.
 */
void timer_init (void);


/**
 * @brief Starts a timer.
 *
 *    @param timer Timer to start.
 *    @param ms Milliseconds for the timer to wait.
 *    @param func Function callback when timer is up or NULL to not use.
 */
void timer_start( int timer, int ms, void (*func)(int) );


/**
 * @brief Stops a timer.
 *
 *    @param timer timer to stop.
 */
void timer_stop( int timer );


#endif /* _TIMER_H */

