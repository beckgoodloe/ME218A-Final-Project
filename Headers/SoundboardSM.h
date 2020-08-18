/****************************************************************************

  Header file for Soundboard SM
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef SoundboardSM_H
#define SoundboardSM_H

#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"

// Public Function Prototypes

bool InitSoundboardSM(uint8_t Priority);
bool PostSoundboardSM(ES_Event_t ThisEvent);
ES_Event_t RunSoundboardSM(ES_Event_t ThisEvent);

typedef enum { SoundboardInitPState, Triggering, Ready2Trigger } SoundboardState_t;

#endif /* SoundboardSm_H */

