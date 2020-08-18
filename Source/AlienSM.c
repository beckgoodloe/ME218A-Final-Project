/****************************************************************************
 Module
   TemplateFSM.c

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
#include "AlienSM.h"
#include "Interaction.h"
#include "MainGameSM.h"

// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "BITDEFS.H"
/*----------------------------- Module Defines ----------------------------*/
#define ALIEN_HIT_THRESHOLD 4
#define ALIEN_FAIL_TIME 10000

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static uint8_t RandomAlien_3(void);
static uint8_t RandomAlien_2(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static AlienState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
static uint8_t AlienHitCount;
static uint8_t LastAlien1ButtonState;
static uint8_t LastAlien2ButtonState;
static uint8_t LastAlien3ButtonState;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitAlienSM

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
bool InitAlienSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
 
  
  // put us into the Initial PseudoState
  CurrentState = AlienInitPState;
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
     PostAlienSM

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
bool PostAlienSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunAlienSM

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
ES_Event_t RunAlienSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  
  switch (CurrentState)
  {
    case AlienInitPState:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
      {
        DeactivateAlien();
        LastAlien1ButtonState = GetAlien1Button();
        LastAlien2ButtonState = GetAlien2Button();
        LastAlien3ButtonState = GetAlien3Button();
        CurrentState = AlienOK;
      }
    }
    break;


    case Alien1Active:
    {
      if (ThisEvent.EventType == Alien1_Button_Hit) {
        AlienHitCount++;
        if(AlienHitCount < ALIEN_HIT_THRESHOLD){
          DimAlienLEDs();
          TurnOffAlien1LED();
          uint8_t NextAlien = RandomAlien_2();
          if (NextAlien == 0) {
            TurnOnAlien3LED();
            CurrentState = Alien3Active;
          }
          else {
            TurnOnAlien2LED();
            CurrentState = Alien2Active;
          }
        }
        else {
          // Stop fail Timer, deactivate alien, reset hit count
          ES_Timer_StopTimer(ALIEN_FAIL_TIMER);
          DeactivateAlien();
          AlienHitCount = 0; // Reset AlienHitCount
          
          //Boost fire
          ES_Event_t Event2Post;
          Event2Post.EventType = Fire_Boost;
          PostMainGameSM(Event2Post);
          
          //Start AlienEventTimer
          ES_Timer_InitTimer(ALIEN_EVENT_TIMER, RandomTime());
          printf("Alien OK now\n\r");
          CurrentState = AlienOK;
        }
      } else if (ThisEvent.EventType == ES_TIMEOUT) {
          if ((ThisEvent.EventParam == FUEL_FAIL_TIMER) | (ThisEvent.EventParam == AIR_FAIL_TIMER) | (ThisEvent.EventParam == ALIEN_FAIL_TIMER) | (ThisEvent.EventParam == SUCCESS_TIMER)) {
            DeactivateAlien();
            AlienHitCount = 0;
            ES_Timer_StopTimer(ALIEN_FAIL_TIMER);
            ES_Timer_StopTimer(ALIEN_EVENT_TIMER);
            CurrentState = AlienOK;
          }
        }
    }
    break;
    
    case Alien2Active:
    {
      if (ThisEvent.EventType == Alien2_Button_Hit) {
        AlienHitCount++;
        if(AlienHitCount < ALIEN_HIT_THRESHOLD){
          DimAlienLEDs();
          TurnOffAlien2LED();
          uint8_t NextAlien = RandomAlien_2();
          if (NextAlien == 0) {
            TurnOnAlien1LED();
            CurrentState = Alien1Active;
          }
          else {
            TurnOnAlien3LED();
            CurrentState = Alien3Active;
          }
        }
        else {
          // Stop fail Timer, deactivate alien, reset hit count
          ES_Timer_StopTimer(ALIEN_FAIL_TIMER);
          DeactivateAlien();
          AlienHitCount = 0; // Reset AlienHitCount
          
          //Boost fire
          ES_Event_t Event2Post;
          Event2Post.EventType = Fire_Boost;
          PostMainGameSM(Event2Post);
          
          //Start AlienEventTimer
          ES_Timer_InitTimer(ALIEN_EVENT_TIMER, RandomTime());
          printf("Alien OK now\n\r");
          CurrentState = AlienOK;
        }
      } else if (ThisEvent.EventType == ES_TIMEOUT) {
          if ((ThisEvent.EventParam == FUEL_FAIL_TIMER) | (ThisEvent.EventParam == AIR_FAIL_TIMER) | (ThisEvent.EventParam == ALIEN_FAIL_TIMER) | (ThisEvent.EventParam == SUCCESS_TIMER)) {
            DeactivateAlien();
            AlienHitCount = 0;
            ES_Timer_StopTimer(ALIEN_FAIL_TIMER);
            ES_Timer_StopTimer(ALIEN_EVENT_TIMER);
            CurrentState = AlienOK;
          }
        }
    }
    break;
    
    case Alien3Active:
    {
      if (ThisEvent.EventType == Alien3_Button_Hit) {
        AlienHitCount++;
        if(AlienHitCount < ALIEN_HIT_THRESHOLD){
          DimAlienLEDs();
          TurnOffAlien3LED();
          uint8_t NextAlien = RandomAlien_2();
          if (NextAlien == 0) {
            TurnOnAlien2LED();
            CurrentState = Alien2Active;
          }
          else {
            TurnOnAlien1LED();
            CurrentState = Alien1Active;
          }
        }
        else {
          // Stop fail Timer, deactivate alien, reset hit count
          ES_Timer_StopTimer(ALIEN_FAIL_TIMER);
          DeactivateAlien();
          AlienHitCount = 0; // Reset AlienHitCount
          
          //Boost fire
          ES_Event_t Event2Post;
          Event2Post.EventType = Fire_Boost;
          PostMainGameSM(Event2Post);
          
          //Start AlienEventTimer
          ES_Timer_InitTimer(ALIEN_EVENT_TIMER, RandomTime());
          printf("Alien OK now\n\r");
          CurrentState = AlienOK;
        }
      } else if (ThisEvent.EventType == ES_TIMEOUT) {
          if ((ThisEvent.EventParam == FUEL_FAIL_TIMER) | (ThisEvent.EventParam == AIR_FAIL_TIMER) | (ThisEvent.EventParam == ALIEN_FAIL_TIMER) | (ThisEvent.EventParam == SUCCESS_TIMER)) {
            DeactivateAlien();
            AlienHitCount = 0;
            ES_Timer_StopTimer(ALIEN_FAIL_TIMER);
            ES_Timer_StopTimer(ALIEN_EVENT_TIMER);
            CurrentState = AlienOK;
          }
        }
    }
    break;
          
    case AlienOK:
    {
      if (ThisEvent.EventType == ES_TIMEOUT) {
        if (ThisEvent.EventParam == ALIEN_EVENT_TIMER) {
          // Post FireDim Event to MainGameSM
          ES_Event_t Event2Post;
          Event2Post.EventType = Fire_Dim;
          PostMainGameSM(Event2Post);
          
          // Init AlienFailTimer
          ES_Timer_InitTimer(ALIEN_FAIL_TIMER, ALIEN_FAIL_TIME);
          
          //Light all alien LEDs
          ActivateAlien();
          printf("Start Alien Attack\n\r");
          
          uint8_t FirstAlien = RandomAlien_3();
          if (FirstAlien == 1) {
            TurnOnAlien1LED();
            CurrentState = Alien1Active;
          } else if (FirstAlien == 2) {
            TurnOnAlien2LED();
            CurrentState = Alien2Active;
          } else {
            TurnOnAlien3LED();
            CurrentState = Alien3Active;
          }
        }
        
        else if ((ThisEvent.EventParam == FUEL_FAIL_TIMER) | (ThisEvent.EventParam == AIR_FAIL_TIMER) | (ThisEvent.EventParam == ALIEN_FAIL_TIMER) | (ThisEvent.EventParam == SUCCESS_TIMER)) {
          DeactivateAlien();
          AlienHitCount = 0;
          ES_Timer_StopTimer(ALIEN_FAIL_TIMER);
          ES_Timer_StopTimer(ALIEN_EVENT_TIMER);
        }
      }
    }
    break;
  }
  
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryAlienSM

 Parameters
     None

 Returns
     TemplateState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
AlienState_t QueryAlienSM(void)
{
  return CurrentState;
}


bool Check4Alien1ButtonHit(void) {
  bool ReturnVal = false;
  ES_Event_t Event2Post;
    
  if (CurrentState == Alien1Active) {
    uint8_t CurrentAlien1ButtonState = GetAlien1Button();
    if (CurrentAlien1ButtonState != LastAlien1ButtonState) {
      if (CurrentAlien1ButtonState == 0){
        printf("alien1buttonhit!\n\r");
        Event2Post.EventType = Alien1_Button_Hit;
        PostAlienSM(Event2Post);
        ReturnVal = true;
      }
    }
    LastAlien1ButtonState = CurrentAlien1ButtonState;
    return ReturnVal;
  }
  else {
    return ReturnVal;
  }
}

bool Check4Alien2ButtonHit(void) {
  bool ReturnVal = false;
  ES_Event_t Event2Post;
    
  if (CurrentState == Alien2Active) {
    uint8_t CurrentAlien2ButtonState = GetAlien2Button();
    if (CurrentAlien2ButtonState != LastAlien2ButtonState) {
      if (CurrentAlien2ButtonState == 0){
        printf("alien2buttonhit!\n\r");
        Event2Post.EventType = Alien2_Button_Hit;
        PostAlienSM(Event2Post);
        ReturnVal = true;
      }
    }
    LastAlien2ButtonState = CurrentAlien2ButtonState;
    return ReturnVal;
  }
  else {
    return ReturnVal;
  }
}

bool Check4Alien3ButtonHit(void) {
  bool ReturnVal = false;
  ES_Event_t Event2Post;
    
  if (CurrentState == Alien3Active) {
    uint8_t CurrentAlien3ButtonState = GetAlien3Button();
    if (CurrentAlien3ButtonState != LastAlien3ButtonState) {
      if (CurrentAlien3ButtonState == 0){
        printf("alien3buttonhit!\n\r");
        Event2Post.EventType = Alien3_Button_Hit;
        PostAlienSM(Event2Post);
        ReturnVal = true;
      }
    }
    LastAlien3ButtonState = CurrentAlien3ButtonState;
    return ReturnVal;
  }
  else {
    return ReturnVal;
  }
}

uint8_t GetAlienHitCount(void) {
  return AlienHitCount;
}


/***************************************************************************
 private functions
 ***************************************************************************/
static uint8_t RandomAlien_3(void) {
  srand(ES_Timer_GetTime());
  return ((rand()%3)+1);
}

static uint8_t RandomAlien_2(void) {
  srand(ES_Timer_GetTime());
  return (rand()%2);
}
