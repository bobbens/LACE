

#ifndef _EVENT_H
#  define _EVENT_H


typedef enum event_type_e {
   EVENT_TYPE_NONE,
   /* SPI Master. */
   EVENT_TYPE_SPI,
   /* Module. */
   EVENT_TYPE_MODULE,
   /* ADC */
   EVENT_TYPE_ADC
} event_type_t;


typedef struct event_spi_s {
   event_type_t type;
   int port;
} event_spi_t;


typedef struct event_module_s {
   event_type_t type;
   int port;
} event_module_t;


typedef union event_u {
   event_type_t type; /**< Type of event. */
   event_spi_t spi;
   event_module_t module;
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
