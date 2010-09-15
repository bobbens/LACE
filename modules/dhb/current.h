

#ifndef CURRENT_H
#  define CURRENT_H


#include <stdint.h>


void current_init (void);
uint16_t current_get( int channel );
void current_startSample (void);


#endif /* CURRENT_H */


