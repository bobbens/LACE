

#ifndef _WMP_H
#  define _WMP_H


#include <stdint.h>


#define WMP_YAW   0  /**< Yaw information. */
#define WMP_PITCH 1  /**< Pitch information. */
#define WMP_ROLL  2  /**< Roll information. */


#define WMP_EVENT_ON    0x53 /**< ID for custom WM+ is on event. */
#define WMP_EVENT_DATA  0x52 /**< ID for custom WM+ data ready event. */
#define WMP_EVENT_ERR   0x51 /**< ID for custom WM+ error event. */


/**
 * @brief Initializes the WM+.
 *
 * Generates a custom event with id WMP_EVENT_ON and data 0x00 when it's complete.
 */
void wmp_init (void);


/**
 * @brief Exits the WM+.
 *
 * Generates a custom event with id WMP_EVENT_ON and data 0x01 when it's complete.
 */
void wmp_exit (void);


/**
 * @brief Starts a conversion on the WM+
 *
 * Generates a custom event with WMP_EVENT_DATA when it's complete.
 */
void wmp_start (void);


/**
 * @brief Gets the current value of the WM+ data.
 *
 * @code
 * uint16_t *data = wmp_data();
 * printf( "Yaw: %d\n", data[ WMP_YAW ] );
 * @endcode
 */
uint16_t* wmp_data (void);


#endif /* _WMP_H */


