/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef FuelSM_H
#define FuelSM_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitFuelSM(uint8_t Priority);
bool PostFuelSM(ES_Event_t ThisEvent);
ES_Event_t RunFuelSM(ES_Event_t ThisEvent);
bool Check4DialMove(void);

typedef enum { FuelInitPState, FuelNeeded, FuelOK } FuelState_t;



#endif /* FuelSM_H */

