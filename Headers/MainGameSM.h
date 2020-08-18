/****************************************************************************

  Header file for Main Game State Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef MainGameSM_H
#define MainGameSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the MainGame states
typedef enum {MainInitPState, Sleep, Welcome, InGame, GameOver} MainGameState_t;

// Public Function Prototypes

bool InitMainGameSM(uint8_t Priority);
bool PostMainGameSM(ES_Event_t ThisEvent);
ES_Event_t RunMainGameSM(ES_Event_t ThisEvent);
MainGameState_t QueryMainGameSM(void);
bool Check4TOT(void);
bool Check4FirestarterHit(void);
uint16_t RandomTime(void);


#endif /* MainGameSM_H */
