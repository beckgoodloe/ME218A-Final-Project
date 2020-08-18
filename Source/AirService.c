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

#include "AirService.h"

/*----------------------------- Module Defines ----------------------------*/
#define AIR_THRESHOLD 20

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
void TurnOnMicLEDs(void);
void TurnOffMicLEDs(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static AirServiceState_t CurrentState;
static uint8_t AirCount;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitAirService

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
bool InitAirService(uint8_t Priority)
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
     PostAirService

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
bool PostAirService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunAirService

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
ES_Event_t RunAirService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  /********************************************
   in here you write your service code
   *******************************************/
  // Two States: AirNeeded and AirOK
    switch(CurrentState){
      case AirNeeded:
        //If ThisEvent.EventType is AirCheckTimer timing out:
          if(AirCount < AIR_THRESHOLD){
            // Increment AirCount by current mic intensity
            // Init AirCheckTimer
          } else {
            // Stop Air Fail Timer
            // Post FireBoost Event to MainGameSM
            // Deactive mic indicator LEDs
            AirCount = 0; // Reset AirCount
            // Init AirEventTimer
            CurrentState = AirOK;
          }
        //endif
        break;
      case AirOK:
        //If ThisEvent.EventType is Air Event Timer timing out
          //Post FireDim Event to MainGameSM
          // Activate Mic indicator LEDS
          // Init AirFailTimer
          // InitAirCheckTimer
          CurrentState = AirNeeded;
        //endif
        break;
    }
        
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
void TurnOnMicLEDs(void){

}
void TurnOffMicLEDs(void){

}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

