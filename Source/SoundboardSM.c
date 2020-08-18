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

#include "SoundboardSM.h"

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
#define MIN_PULSE 500
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static SoundboardState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitSoundboardSM

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
bool InitSoundboardSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  CurrentState = SoundboardInitPState;
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
     PostSoundboardSM

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
bool PostSoundboardSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunSoundboardSM

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
ES_Event_t RunSoundboardSM(ES_Event_t ThisEvent)
{
  SoundboardState_t NextState;
  NextState = CurrentState;
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  /********************************************
   in here you write your service code
   *******************************************/
  //Two states: Triggering and Playing (can be configured for more pins later)  
  switch(CurrentState){
    case SoundboardInitPState:
      if (ThisEvent.EventType == ES_INIT) {
        NextState = Ready2Trigger;
        printf("transirtion to ready2trigger\n\r");
      }
      break;
    
    case Triggering:
      if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == SOUNDBOARD_TIMER){
        //Done triggering, leave pin high until ready to trigger another sound
        HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT6HI | BIT7HI;
        HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + ALL_BITS)) |= BIT4HI;
        ES_Timer_StopTimer(SOUNDBOARD_TIMER);
        NextState = Ready2Trigger;
      }
      break;
      
    case Ready2Trigger:
//      if(ThisEvent.EventType == Success_Sound){
//        //start pulse timer
//        printf("success SOUND NOISE LOUD\n\r");
//        ES_Timer_InitTimer(SOUNDBOARD_TIMER, MIN_PULSE);
//        HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT6LO; 
//        NextState=Triggering;
//      }
      if(ThisEvent.EventType == Fail_Sound){
        printf("fail SOUND NOISE LOUD\n\r");
        ES_Timer_InitTimer(SOUNDBOARD_TIMER, MIN_PULSE);
        HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT7LO;
        NextState=Triggering;        
      }
      else if(ThisEvent.EventType == Success_Sound){
        printf("success SOUND NOISE LOUD\n\r");
        ES_Timer_InitTimer(SOUNDBOARD_TIMER, MIN_PULSE);
        HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT4LO; 
        NextState=Triggering;
      }
      
      break;
    }
  CurrentState = NextState;
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

