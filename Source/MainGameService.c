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

#include "MainGameService.h"

/*----------------------------- Module Defines ----------------------------*/
#define FIRE_START_THRESHOLD 19
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static MainGameState_t CurrentState;
static uint16_t FirestarterCount;
static uint16_t TimeRemaining;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTemplateService

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
bool InitMainGameService(uint8_t Priority)
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
     PostMainGameService

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
bool PostMainGameService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMainGameService

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
ES_Event_t RunMainGameService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  /********************************************
   in here you write your service code
   *******************************************/
  //States: Welcome, InGame, GameOver, Sleep
  switch(CurrentState){
    case Welcome:
      if(ThisEvent.EventType == Firestarter_Hit){
        if (FirestarterCount < FIRE_START_THRESHOLD){
          FirestarterCount++;
          printf("Inc Fire");
        } else {
         // Turn on FireLEDs
         // Turn on Fan to 100%
         // Init Alien Event Timer, Fuel Event Timer, Success Timer, Kitchen Timer, 
         // Post Initial Air event Timer Timeout to AirSM
        printf("Enough Fire. Start Game");
        CurrentState = InGame;
        }
      }
      break;
    case InGame:
      if(ThisEvent.EventType == ES_TIMEOUT){
        switch(ThisEvent.EventParam){
          case AIR_FAIL_TIMER:
            // play sad sounds
            // turn off fire
            // return TOT
            // start game over timer
            printf("Air Failure");
            CurrentState = GameOver;
            break;
          case ALIEN_FAIL_TIMER:
            // play sad sounds
            // turn off fire
            // return TOT
            // start game over timer
            printf("Alien Failure");
            CurrentState = GameOver;
            break;
          case FUEL_FAIL_TIMER:
            // play sad sounds
            // turn off fire
            // return TOT
            // start game over timer
            printf("Fuel Failure");
            CurrentState = GameOver;
            break;
          case SUCCESS_TIMER:
            // play happy sounds
            // turn on fire TO MAX
            // return TOT
            // give potato
            // start game over timer
            printf("You win");
            CurrentState = GameOver;
            break;
          case KITCHEN_TIMER:
            TimeRemaining--;
            printf("Dec Kitchen Timer");
            break;
        }
      }
      if(ThisEvent.EventType == Fire_Boost){
        // Boost Fire and Fan PWM
        printf("FireBoost");
      }
      if (ThisEvent.EventType == Fire_Dim){
        // Reduce Fire and Fan PWM
        printf("FireDim");
      }
      break;
    case GameOver:
      if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == GAMEOVER_TIMER){
        // turn off LEDs and fan
        printf("Fire and LED off");
        CurrentState = Sleep;
      }
      break;
    case Sleep:
      if(ThisEvent.EventType == TOT_Detected){
        // turn on fire starter LEDs 
        //dimly light fire LEDs
        printf("StartGame out of Sleep");
        CurrentState = Welcome;
      }
      break;
  
  }   
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

