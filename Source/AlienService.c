/****************************************************************************
 Module
   TemplateService.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TemplateService.h"

#include "AlienService.h"

/*----------------------------- Module Defines ----------------------------*/
#define ALIEN_HIT_THRESHOLD 9
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

void ActivateAlien(void);
void DimAlienLEDs(void);
void DeactivateAlien(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t AlienHitCount;

static AlienServiceState_t CurrentState;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitAlienService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
****************************************************************************/
bool InitAlienService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService(MyPriority, ThisEvent) == true)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/****************************************************************************
 Function
     PostAlienService

 Parameters
     EF_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostAlienService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunAlienService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes

 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunAlienService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  /********************************************
   in here you write your service code
   *******************************************/
  //Two states for this service: AlienAttack and AlienOK
    switch(CurrentState){
      case AlienAttack:
        //if ThisEvent.EventType is the ALIEN BUTTON HIT
          if(AlienHitCount < ALIEN_HIT_THRESHOLD){
            AlienHitCount++;
            // Dim Alien LEDs by 10%
          } else {
            //Stop AlienFailTimer
            //Deactivate Alien LEDs
            AlienHitCount = 0; // Reset AlienHitCount
            //Post FireBoost Event to MainGameSM
            //Start AlienEventTimer
            printf("Alien OK now");
            CurrentState = AlienOK;
          }
        //endif
        break;
      case AlienOK:
        //If ThisEvent.EventType is the ALIEN EVENT TIMER TIMEOUT:
          // Post FireDim Event to MainGameSM
          // Init AlienFailTimer
          //Light all alien LEDs
          printf("Start Alien Attack");
          CurrentState = AlienAttack;
        //endif
        break;
    }

  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

void ActivateAlien(void){

}

void DimAlienLEDs(void){

}

void DeactivateAlien(void){

}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

