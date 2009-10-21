

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
   /* Module. */
   EVENT_TYPE_MODULE, /**< Module subsystem event. */
   /* ADC */
   EVENT_TYPE_ADC /**< ADC event. */
} event_type_t;


/**
 * @brief SPI subsystem event.
 */
typedef struct event_spi_s {
   event_type_t type; /**< Type of the event. */
   int port; /**< SPI port generating event. */
} event_spi_t;


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
 * @brief All the events.
 */
typedef union event_u {
   event_type_t type; /**< Type of the event. */
   event_spi_t spi; /**< SPI event. */
   event_module_t module; /**< Module event. */
   event_adc_t adc; /**< ADC event. */
} event_t;


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
