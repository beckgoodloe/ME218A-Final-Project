/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef AirSM_H
#define AirSM_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitAirSM(uint8_t Priority);
bool PostAirSM(ES_Event_t ThisEvent);
ES_Event_t RunAirSM(ES_Event_t ThisEvent);

typedef enum { AirInitPState, AirNeeded, AirOK } AirState_t;


#endif /* AirSM_H */

