/****************************************************************************
 Module
   MainGameSM.c

 Revision
   0.0.1

 Description
   Main Game State Machine

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/10/19 11:12 tnc      kbhit framework test

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include <inttypes.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MainGameSM.h"
#include "FuelSM.h"
#include "AirSM.h"
#include "AlienSM.h"
#include "Interaction.h"
#include "SoundboardSM.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

/*----------------------------- Module Defines ----------------------------*/
#define GAME_LENGTH 50000
#define FIRESTARTER_HITS_REQD 25

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static MainGameState_t CurrentState;
static uint8_t LastTOTState = 0;
static uint8_t FireStarterCount;
static uint8_t EstTimeRemaining;
static uint8_t LastFirestarterButtonState = 0;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMainGameSM

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
bool InitMainGameSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = MainInitPState;
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

bool Check4TOT(void) {
  bool ReturnVal = false;
  ES_Event_t Event2Post;
  uint8_t CurrentTOTState = CheckTOT();
  if (CurrentTOTState != LastTOTState) {
    if (CurrentTOTState == BIT3HI){
      Event2Post.EventType = TOT_Detected;
      PostMainGameSM(Event2Post);
      ReturnVal = true;
    }
  }
  LastTOTState = CurrentTOTState;
  return ReturnVal;
}


bool Check4FirestarterHit(void) {
  bool ReturnVal = false;
  ES_Event_t Event2Post;
  if (CurrentState == Welcome) {
    uint8_t CurrentFirestarterButtonState = GetFirestarterButton();
    if (CurrentFirestarterButtonState != LastFirestarterButtonState) {
      if (CurrentFirestarterButtonState == 0){
        Event2Post.EventType = Firestarter_Hit;
        PostMainGameSM(Event2Post);
        ReturnVal = true;
      }
    }
    LastFirestarterButtonState = CurrentFirestarterButtonState;
  return ReturnVal;
  }
  else {
    return ReturnVal;
  }
}
  

/****************************************************************************
 Function
     PostMainGameSM

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
bool PostMainGameSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMainGameSM

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
ES_Event_t RunMainGameSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  MainGameState_t NextState = CurrentState;
  
  switch (CurrentState)
  {
    case MainInitPState:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
      {
        printf("initializing main I/O\n\r");
        InitInteraction();
        CloseTOTServo();
        FireOff();
//        
//        TurnOnAlien1LED();
//        TurnOnAlien2LED();
//        TurnOnAlien3LED();
//      
        NextState = Sleep;
      }
    }
    break;

    case Sleep:
    {
      switch (ThisEvent.EventType)
      {
        case TOT_Detected:
        {
          TurnOnFireStarterLEDs();
          FireStarterCount = 0;
          NextState = Welcome; 
          
        }
        break;
      }
    }
    break;
    
    case Welcome:
    {
      switch (ThisEvent.EventType)
      {
        case Firestarter_Hit:
        {   
          printf("Firestarter Hit Detected\n\r");
          FireStarterCount++;
          if (FireStarterCount % 5 == 0) {
            BoostFire();
          }
          if (FireStarterCount >= FIRESTARTER_HITS_REQD) {
            MaxFire();
            TurnOffFireStarterLEDs();
            // Initialize FuelSM from InitPState
            PostFuelSM(ThisEvent);
            PostAirSM(ThisEvent);
            //Start AlienEvent, FuelEvent, success timers
            ES_Timer_InitTimer(ALIEN_EVENT_TIMER, RandomTime());
            ES_Timer_InitTimer(FUEL_EVENT_TIMER, RandomTime());
            ES_Timer_InitTimer(SUCCESS_TIMER, GAME_LENGTH);
            
            //Post Air Event 
            ES_Event_t Event2Post;
            Event2Post.EventType = ES_TIMEOUT;
            Event2Post.EventParam = AIR_EVENT_TIMER;
            PostAirSM(Event2Post);

            // Start Kitchen Timer
            EstTimeRemaining = 50;
            ES_Timer_InitTimer(KITCHEN_TIMER, 1000);
            NextState = InGame;
          }
        }
        break;
      }
    }
    break;
    
    case InGame:
    {
      switch (ThisEvent.EventType)
      {
        case ES_TIMEOUT:
        {
          switch(ThisEvent.EventParam)
          {
            case KITCHEN_TIMER:
            {
              EstTimeRemaining--;
              //printf("Time Remaining = %" PRIu8 "\n\r",EstTimeRemaining);
              MoveKitchenTimer(EstTimeRemaining);
              ES_Timer_InitTimer(KITCHEN_TIMER, 1000);
            }
            break;
            
            case SUCCESS_TIMER:
            {
              printf("POTATO COOKED!!\n\r");
              ES_Event_t Event2Post;
              Event2Post.EventType = Success_Sound;
              PostSoundboardSM(Event2Post);
              MoveKitchenTimer(0);
              MaxFire();
              
              // Post success event to all other state machines
              PostFuelSM(ThisEvent);
              PostAirSM(ThisEvent);
              PostAlienSM(ThisEvent);
              
              //Start GameOver Timer and Transition to GameOver state
              ES_Timer_InitTimer(GAMEOVER_TIMER, 10000);
              OpenTOTServo();
              NextState = GameOver;
            }
            break;
            
            case AIR_FAIL_TIMER:
            {
              printf("FIRE PUT OUT :(:(\n\r");
              ES_Event_t Event2Post;
              Event2Post.EventType = Fail_Sound;
              PostSoundboardSM(Event2Post);
              MoveKitchenTimer(0);
              FireOff();
              
              // Post failure event to all other state machines
              PostFuelSM(ThisEvent);
              PostAirSM(ThisEvent);
              PostAlienSM(ThisEvent);
              //Start GameOver Timer and Transition to GameOver state
              ES_Timer_InitTimer(GAMEOVER_TIMER, 10000);\
              OpenTOTServo();
              NextState = GameOver;
            }
            break;
            
            case ALIEN_FAIL_TIMER:
            {
              printf("FIRE PUT OUT :(:(\n\r");
              ES_Event_t Event2Post;
              Event2Post.EventType = Fail_Sound;
              PostSoundboardSM(Event2Post);
              MoveKitchenTimer(0);
              FireOff();
              // Post failure event to all other state machines
              PostFuelSM(ThisEvent);
              PostAirSM(ThisEvent);
              PostAlienSM(ThisEvent);
              // Start GameOver Timer and Transition to GameOver state
              ES_Timer_InitTimer(GAMEOVER_TIMER, 10000);
              OpenTOTServo();
              NextState = GameOver;
            }
            break;
            
            case FUEL_FAIL_TIMER:
            {
              printf("FIRE PUT OUT :(:(\n\r");
              ES_Event_t Event2Post;
              Event2Post.EventType = Fail_Sound;
              PostSoundboardSM(Event2Post);
              MoveKitchenTimer(0);
              FireOff();
              // Post failure event to all other state machines
              PostFuelSM(ThisEvent);
              PostAirSM(ThisEvent);
              PostAlienSM(ThisEvent);
              //Start GameOver Timer and Transition to GameOver state
              ES_Timer_InitTimer(GAMEOVER_TIMER, 10000);
              OpenTOTServo();
              NextState = GameOver;
            }
            break;
          }
        }
        break;
      
        case Fire_Boost:
        {
          BoostFire();
        }
        break;
        
        case Fire_Dim:
        {
          DimFire();
        }
        break;
      }
    }
    break;
    
    case GameOver:
    {
      switch (ThisEvent.EventType)
      {
        case ES_TIMEOUT:
        {
          switch (ThisEvent.EventParam)
          {
            case GAMEOVER_TIMER:
            {
              FireOff();
              CloseTOTServo();
              NextState = Sleep;
            }
            break;
          }
        }
        break;
      }
    }
    break;
  }
  
  CurrentState = NextState;
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryMainGameSM

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
MainGameState_t QueryMainGameSM(void)
{
  return CurrentState;
}

uint16_t RandomTime(void) {
  srand(ES_Timer_GetTime());
  return ((rand()%5000) + 2000);
}

/***************************************************************************
 private functions
 ***************************************************************************/

