

#ifndef _EVENT_H
#  define _EVENT_H


#include <stdint.h>


/**
 * @brief Event types.
 */
typedef enum event_type_e {
   EVENT_TYPE_NONE, /**< No valid event. */
   /* SPI Master. */
   EVENT_TYPE_SPI, /**< SPI subsystem event. */
   /* I2C Master. */
   EVENT_TYPE_I2C, /**< I2C subsystem event. */
   /* Module. */
   EVENT_TYPE_MODULE, /**< Module subsystem event. */
   /* ADC */
   EVENT_TYPE_ADC, /**< ADC event. */
   /* Timer. */
   EVENT_TYPE_TIMER, /**< Timer event. */
   /* Custom. */
   EVENT_TYPE_CUSTOm, /**< Custom event. */
   /* Sentinal for maximum. */
   EVENT_TYPE_MAX /**< Maximum amount of events. */
} event_type_t;


/**
 * @brief SPI subsystem event.
 */
typedef struct event_spi_s {
   event_type_t type; /**< Type of the event. */
   int port; /**< SPI port generating event. */
} event_spi_t;


/**
 * @brief I2C subsystem event.
 */
typedef struct event_i2c_s {
   event_type_t type; /**< Type of the event. */
   int address; /**< Address communicating with. */
   int rw; /**< Whether reading or writing. */
   int ok; /**< WHether or not it was completed fully and successfully. */
} event_i2c_t;


/**
 * @brief Module subsystem event.
 */
typedef struct event_module_s {
   event_type_t type; /**< Type of the event. */
   int port; /**< Module generating the event. */
} event_module_t;


/**
 * @brief ADC event.
 */
typedef struct event_adc_s {
   event_type_t type; /**< Type of the event. */
   int channel; /**< ADC port generating the event. */
} event_adc_t;


/**
 * @brief Timer event.
 */
typedef struct event_timer_s {
   event_type_t type; /**< Type of the event. */
   int timer; /**< Timer generating the event. */
} event_timer_t;


/**
 * @brief Custom event.
 */
typedef struct event_custom_s {
   event_type_t type; /**< Typo of the event. */
   int id; /**< Custom identifier for the event. */
} event_custom_t;



/**
 * @brief All the events.
 */
typedef union event_u {
   event_type_t type; /**< Type of the event. */
   event_spi_t spi; /**< SPI event. */
   event_i2c_t i2c; /**< I2C event. */
   event_module_t module; /**< Module event. */
   event_adc_t adc; /**< ADC event. */
   event_timer_t timer; /**< Timer event. */
} event_t;


typedef int(*event_callback_t)(event_t*);


/**
 * @brief Initializes the event subsystem.
 */
void event_init (void);


/**
 * @brief Sets a callback on a certain event.
 *
 * The callback function takes the event recieving as a parameter. It must
 *  return 0 to continue generating the event or 1 to destroy the event.
 *
 *    @param type Type of event to set callback on.
 *    @param func Callback function.
 */
void event_setCallback( event_type_t type, event_callback_t func );


/**
 * @brief Pushes an event onto the event stack.
 *
 *    @param evt Event to push onto stack.
 */
void event_push( event_t *evt );


/**
 * @brief Polls for events.
 *
 *    @param evt Event to fill.
 *    @return 1 if an event was found, 0 if no events on stack.
 */
int event_poll( event_t *evt );


#endif /* _EVENT_H */
