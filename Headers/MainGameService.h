/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef MainGame_H
#define MainGame_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitMainGameService(uint8_t Priority);
bool PostMainGameService(ES_Event_t ThisEvent);
ES_Event_t RunMainGameService(ES_Event_t ThisEvent);

typedef enum { Welcome, InGame, GameOver, Sleep} MainGameState_t;


#endif /* MainGame_H */

