/****************************************************************************
 Module
   FuelSM.c

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
#include "FuelSM.h"
#include "Interaction.h"
#include "MainGameSM.h"
#include <inttypes.h>


/*----------------------------- Module Defines ----------------------------*/
#define SCALING_FACTOR 0.5 //arbitrary value, for now
#define FUEL_FAIL_TIME 7000
#define DIAL_TOLERANCE 75
#define DIAL_MAX 3800
#define THRESHOLD_4 3500
#define THRESHOLD_3 2800
#define THRESHOLD_2 1900
#define THRESHOLD_1 800
#define BLINK_LENGTH 500

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static uint8_t GetCorrectPositionFromDial(uint16_t CurrentDialPosition);
static bool InRange(uint16_t DialPosition);
static uint8_t GetNewLEDPosition(void);
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static FuelState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
static uint8_t CorrectLEDPosition;
static uint16_t LastDialPosition;
static bool LightsCurrentlyOn;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFuelSM

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
bool InitFuelSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = FuelInitPState;
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
     PostFuelSM

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
bool PostFuelSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFuelSM

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
ES_Event_t RunFuelSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case FuelInitPState:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == Firestarter_Hit)    // only respond to ES_Init
      {
        // Set the servo position to the dial position
        LightsCurrentlyOn = true;
        LastDialPosition = GetDialPosition();
        MoveFuelServo(LastDialPosition);

        // Ensure fuel lights at same position as dial position.
        CorrectLEDPosition = GetCorrectPositionFromDial(LastDialPosition);
        SetFuelLEDs(CorrectLEDPosition);
        CurrentState = FuelOK;
      }
    }
    break;

    case FuelNeeded:
    {
      switch(ThisEvent.EventType)
      {
        case Dial_Move:
        {
          uint16_t CurrentDialPosition = ThisEvent.EventParam;
          if (InRange(CurrentDialPosition)) {
            // Make sure LEDs are on
            SetFuelLEDs(CorrectLEDPosition);
            LightsCurrentlyOn = true;
            // update servo position to current position
            MoveFuelServo(CurrentDialPosition);
            // set last dial position to pot_value * scaling factor
            LastDialPosition = CurrentDialPosition;
            // stop fuelfailtimer
            ES_Timer_StopTimer(FUEL_FAIL_TIMER);
            // stop blink timer
            ES_Timer_StopTimer(BLINK_TIMER);
            // init fuel event timer
            ES_Timer_InitTimer(FUEL_EVENT_TIMER, RandomTime());
            printf("Enough Fuel\n");
            // Post FireBoost Event
            ES_Event_t Event2Post;
            Event2Post.EventType = Fire_Boost;
            PostMainGameSM(Event2Post);
            CurrentState = FuelOK;
          }
          else {
            // update servo position to current position
            MoveFuelServo(CurrentDialPosition);
            // set last dial position to pot_value * scaling factor
            LastDialPosition = CurrentDialPosition;
            printf("Still out of range\n\r");
          }
        }
        break;
        
        case ES_TIMEOUT:
        {
          if ((ThisEvent.EventParam == FUEL_FAIL_TIMER) | (ThisEvent.EventParam == AIR_FAIL_TIMER) | (ThisEvent.EventParam == ALIEN_FAIL_TIMER) | (ThisEvent.EventParam == SUCCESS_TIMER)) {
            ES_Timer_StopTimer(FUEL_FAIL_TIMER);
            ES_Timer_StopTimer(FUEL_EVENT_TIMER);
            ES_Timer_StopTimer(BLINK_TIMER);
            TurnOffFuelLEDs();
            CurrentState = FuelInitPState;
            ES_Event_t Event2Post;
            Event2Post.EventType = ES_INIT;
            PostFuelSM(Event2Post);
          }
          else if ((ThisEvent.EventParam == BLINK_TIMER) & LightsCurrentlyOn) {
            TurnOffFuelLEDs();
            LightsCurrentlyOn = false;
            ES_Timer_InitTimer(BLINK_TIMER, BLINK_LENGTH);
          }
          else if ((ThisEvent.EventParam == BLINK_TIMER) & !LightsCurrentlyOn) {
            SetFuelLEDs(CorrectLEDPosition);
            LightsCurrentlyOn = true;
            ES_Timer_InitTimer(BLINK_TIMER, BLINK_LENGTH);
          }
        }
        break;
      }
    }
    break;
      
    case FuelOK:
    {
      switch(ThisEvent.EventType)
      {
        case Dial_Move:
        {
          uint16_t CurrentDialPosition = ThisEvent.EventParam;
          if (!InRange(CurrentDialPosition)) {
            // update servo position to current position
            MoveFuelServo(CurrentDialPosition);
            // set last dial position to pot_value * scaling factor
            LastDialPosition = CurrentDialPosition;
            // Post firedim event to MainGameSM
            ES_Event_t Event2Post;
            Event2Post.EventType = Fire_Dim;
            PostMainGameSM(Event2Post);
            // Stop Fuel Event Timer
            ES_Timer_StopTimer(FUEL_EVENT_TIMER);
            // Start Fuel Fail Timer
            ES_Timer_InitTimer(FUEL_FAIL_TIMER, FUEL_FAIL_TIME);
            // Start Blink Timer
            ES_Timer_InitTimer(BLINK_TIMER, BLINK_LENGTH);
            // Transition to FuelNeeded state
            printf("Moved out of range, fuel needed\n\r");
            CurrentState = FuelNeeded;
          }
          else {
            // if still in range, just update servo position
            // update servo position to current position
            MoveFuelServo(CurrentDialPosition);
            // set last dial position to pot_value * scaling factor
            LastDialPosition = CurrentDialPosition;
          }
        }
        break;
        
        case ES_TIMEOUT:
        {
          if (ThisEvent.EventParam == FUEL_EVENT_TIMER) {
            // Post Firedim event to MainGameSM
            ES_Event_t Event2Post;
            Event2Post.EventType = Fire_Dim;
            PostMainGameSM(Event2Post);
            // Start Fuel_Fail timer
            ES_Timer_InitTimer(FUEL_FAIL_TIMER, FUEL_FAIL_TIME);
            // Set new correct position, light proper LEDs
            CorrectLEDPosition = GetNewLEDPosition();
            SetFuelLEDs(CorrectLEDPosition);
            // Set new state to FuelNeeded
            ES_Timer_InitTimer(BLINK_TIMER, BLINK_LENGTH);
            CurrentState = FuelNeeded;
            printf("Need Fuel again\n\r");
          }
          else if ((ThisEvent.EventParam == FUEL_FAIL_TIMER) | (ThisEvent.EventParam == AIR_FAIL_TIMER) | (ThisEvent.EventParam == ALIEN_FAIL_TIMER) | (ThisEvent.EventParam == SUCCESS_TIMER)) {
            ES_Timer_StopTimer(FUEL_FAIL_TIMER);
            ES_Timer_StopTimer(FUEL_EVENT_TIMER);
            TurnOffFuelLEDs();
            CurrentState = FuelInitPState;
            ES_Event_t Event2Post;
            Event2Post.EventType = ES_INIT;
            PostFuelSM(Event2Post);
          }
        }
        break;
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
     FuelState_t The current state of the Template state machine

 Description
     returns the current state of the Template state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
FuelState_t QueryFuelSM(void)
{
  return CurrentState;
}

bool Check4DialMove(void) {
  bool ReturnVal = false;
  MainGameState_t MainState = QueryMainGameSM();
  if ((MainState == Welcome) | (MainState == InGame)) {
    ES_Event_t Event2Post;
    uint16_t CurrentDialPosition = GetDialPosition();
    if ((CurrentDialPosition > (LastDialPosition + DIAL_TOLERANCE)) | (CurrentDialPosition < (LastDialPosition - DIAL_TOLERANCE))) {
      //printf("Current Dial Pos: %u \r\n Last Dial Pos: %u \r\n", CurrentDialPosition, LastDialPosition);
      Event2Post.EventType = Dial_Move;
      Event2Post.EventParam = CurrentDialPosition;
      PostFuelSM(Event2Post);
      LastDialPosition = CurrentDialPosition;
      ReturnVal = true;
      }
  }
  return ReturnVal;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static uint8_t GetCorrectPositionFromDial(uint16_t DialPosition) {
  if (DialPosition < (THRESHOLD_1)) {
    return 1;
  } else if (DialPosition < (THRESHOLD_2)) {
    return 2;
  } else if (DialPosition < (THRESHOLD_3)) {
    return 3;
  } else if (DialPosition < (THRESHOLD_4)) {
    return 4;
  } else {
    return 5;
  }
}

static bool InRange(uint16_t DialPosition) {
  bool ReturnVal = false;
  if ((DialPosition < (THRESHOLD_1)) & (CorrectLEDPosition == 1)) {
    ReturnVal = true;
  } else if ((DialPosition < (THRESHOLD_2)) & (DialPosition >= THRESHOLD_1) & (CorrectLEDPosition == 2)) {
    ReturnVal = true;
  } else if ((DialPosition < (THRESHOLD_3)) & (DialPosition >= THRESHOLD_2) & (CorrectLEDPosition == 3)) {
    ReturnVal = true;
  } else if ((DialPosition < (THRESHOLD_4)) & (DialPosition >= THRESHOLD_3) & (CorrectLEDPosition == 4)) {
    ReturnVal = true;
  } else if ((DialPosition > THRESHOLD_4) & (CorrectLEDPosition == 5)) {
    ReturnVal = true;
  }
  return ReturnVal;
}

// Returns new LED position from 1 to 5 
static uint8_t GetNewLEDPosition(void) {
  uint8_t NewPosition;
  srand(ES_Timer_GetTime());
  NewPosition = ((rand()%5) + 1);
  while(NewPosition == CorrectLEDPosition) {
    NewPosition = ((rand()%5) + 1);
  }
  return NewPosition;
}
