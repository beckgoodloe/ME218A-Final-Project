/****************************************************************************
 Module
   AirFSM.c

 Revision
   1.0.1

 Description
   This is a template file for implementing flat state machines under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "AirSM.h"
#include "Interaction.h"
#include "MainGameSM.h"
#include <inttypes.h>


/*----------------------------- Module Defines ----------------------------*/
#define AIR_THRESHOLD 10000
#define AIR_FAIL_TIME 20000
#define CHECK_TIME 200

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static AirState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
static uint16_t AirCount;
static uint16_t ZeroIntensity;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitAirSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitAirSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = AirInitPState;
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
     PostAirSM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostAirSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunAirSM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunAirSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case AirInitPState:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
      {
        AirCount = 0;
        TurnOffAirLEDs();
        ZeroIntensity = GetMicIntensity();
        printf("zerovalue = %" PRIu16 "\n\r", ZeroIntensity);
        CurrentState = AirOK;
      }
    }
    break;

    case AirNeeded:
    {
      if (ThisEvent.EventType == ES_TIMEOUT) {
        if (ThisEvent.EventParam == AIR_CHECK_TIMER) {
          if(AirCount < AIR_THRESHOLD){
            //increment Air count by mic intensity, start air check timer
            uint16_t MicIntensity = GetMicIntensity();
            AirCount += abs(ZeroIntensity-MicIntensity);
            printf("Aircount: %" PRIu16 "\n\r",AirCount);
            printf("Intensity: %" PRIu16 "\n\r",MicIntensity);
            printf("Zero: %" PRIu16 "\n\r",ZeroIntensity);

            ES_Timer_InitTimer(AIR_CHECK_TIMER, CHECK_TIME);
          } else {
            // Stop Air Fail Timer
            ES_Timer_StopTimer(AIR_FAIL_TIMER);
            // Post FireBoost Event to MainGameSM
            ES_Event_t Event2Post;
            Event2Post.EventType = Fire_Boost;
            PostMainGameSM(Event2Post);
            // Deactivate mic indicator LEDs
            TurnOffAirLEDs();
            // Reset AirCount
            AirCount = 0; 
            // Init AirEventTimer
            ES_Timer_InitTimer(AIR_EVENT_TIMER, RandomTime());
            CurrentState = AirOK;
          }
        }
        else if ((ThisEvent.EventParam == FUEL_FAIL_TIMER) | (ThisEvent.EventParam == AIR_FAIL_TIMER) | (ThisEvent.EventParam == ALIEN_FAIL_TIMER) | (ThisEvent.EventParam == SUCCESS_TIMER)) {
          TurnOffAirLEDs();
          ES_Timer_StopTimer(AIR_FAIL_TIMER);
          ES_Timer_StopTimer(AIR_EVENT_TIMER);
          AirCount = 0;
          CurrentState = AirOK;
        }
      }
    }
    break;
          
    case AirOK:
    {
      if (ThisEvent.EventType == ES_TIMEOUT) {
        if (ThisEvent.EventParam == AIR_EVENT_TIMER) {
          //Post FireDim Event to MainGameSM          
          ES_Event_t Event2Post;
          Event2Post.EventType = Fire_Dim;
          PostMainGameSM(Event2Post);
          
        // Activate Mic indicator LEDS
          TurnOnAirLEDs();
          AirCount = 0;
        // Init AirFailTimer
          ES_Timer_InitTimer(AIR_FAIL_TIMER, AIR_FAIL_TIME);
        // InitAirCheckTimer
          ES_Timer_InitTimer(AIR_CHECK_TIMER, CHECK_TIME);
          CurrentState = AirNeeded;
        }
        else if ((ThisEvent.EventParam == FUEL_FAIL_TIMER) | (ThisEvent.EventParam == AIR_FAIL_TIMER) | (ThisEvent.EventParam == ALIEN_FAIL_TIMER) | (ThisEvent.EventParam == SUCCESS_TIMER)) {
          TurnOffAirLEDs();
          ES_Timer_StopTimer(AIR_FAIL_TIMER);
          ES_Timer_StopTimer(AIR_EVENT_TIMER);
          AirCount = 0; 
          CurrentState = AirOK;
        }
      }
      else if (ThisEvent.EventType == Firestarter_Hit) {
        printf("zeroed");
        ZeroIntensity = GetMicIntensity();
      }
    }
    break;
  }
  
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryTemplateSM

 Parameters
     None

 Returns
     AirState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
AirState_t QueryAirSM(void)
{
  return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/

