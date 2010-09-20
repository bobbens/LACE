

#ifndef CURRENT_H
#  define CURRENT_H


#include <stdint.h>


void current_init (void);
void current_get( uint8_t *out );
void current_startSample (void);


#endif /* CURRENT_H */


