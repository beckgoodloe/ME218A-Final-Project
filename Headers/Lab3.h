/****************************************************************************
 
  Header file for template service 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef Lab3_H
#define Lab3_H

#include "ES_Types.h"
#include "ES_Events.h"

// Public Function Prototypes

bool Lab3Init ( uint8_t Priority );
bool PostLab3( ES_Event_t ThisEvent );
ES_Event_t Lab3Run( ES_Event_t ThisEvent );


#endif /* Lab3_H */

