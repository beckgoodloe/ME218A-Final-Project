/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef AlienSM_H
#define AlienSM_H

#include "ES_Types.h"
#include "ES_Events.h"

// Public Function Prototypes

bool InitAlienSM(uint8_t Priority);
bool PostAlienSM(ES_Event_t ThisEvent);
ES_Event_t RunAlienSM(ES_Event_t ThisEvent);
bool Check4Alien1ButtonHit(void);
bool Check4Alien2ButtonHit(void);
bool Check4Alien3ButtonHit(void);
uint8_t GetAlienHitCount(void);

typedef enum { AlienInitPState, AlienOK, Alien1Active, Alien2Active, Alien3Active } AlienState_t;


#endif /* AlienSM_H */

