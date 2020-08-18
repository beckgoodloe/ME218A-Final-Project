/****************************************************************************
 Module
   LCDService.c

 Revision
   1.0.1

 Description
   This is the first service for the Test Harness under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/25/17 13:56 jec      added comments about where to init deferal que and
                         fixed bad ONE_SEC definition left over from HC12
 10/19/16 13:24 jec      added comments about where to add deferal and recall
 01/12/15 21:47 jec      converted to LCD module for lab 3
 11/02/13 17:21 jec      added exercise of the event deferral/recall module
 08/05/13 20:33 jec      converted to test harness service
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this service
*/
#include "LCDService.h"

/* include header files for hardware access
*/
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

/* include header files for the framework
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

/* include header files for the other modules in Lab3 that are referenced
*/
#include "LCD_Write.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static LCDState_t CurrentState = InitPState;

// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[3+1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitLCDService

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
bool InitLCDService ( uint8_t Priority )
{
  ES_Event_t ThisEvent;
  
  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  LCD_HWInit();
//Put us into the initial pseudo-state to set up for the initial transition
	CurrentState = InitPState;
// set up the short timer for inter-command timings, only using 1 short timer
  ES_ShortTimerInit(SHORT_TIMER_UNUSED, MyPriority );
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {  
    return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostLCDService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostLCDService( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunLCDService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunLCDService( ES_Event_t ThisEvent )
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  uint16_t DelayTime ;
  
  switch (CurrentState){
    case InitPState :
      if ( ThisEvent.EventType == ES_INIT ){
        // initialize the LCD hardware
        LCD_HWInit();
        // initialize the deferal que
        ES_InitQueue(DeferralQueue, ARRAY_SIZE(DeferralQueue));
        // take the first initialization step and get required delay time
        DelayTime = LCD_TakeInitStep();
        // start the timer for that delay
        ES_ShortTimerStart(TIMER_B, DelayTime);
        // move to the Initializing state
        CurrentState = Initializing;
      }
      break;
    case Initializing :
      if( ThisEvent.EventType == ES_SHORT_TIMEOUT ){
        // take the next LCD initialization step and get the next delay time
        DelayTime = LCD_TakeInitStep();
        // if there are more steps to the initialization, then start the timer
        if (DelayTime > 0) {
          ES_ShortTimerStart(TIMER_B, DelayTime);
        }
        // else move to the Waiting2Write state
        else {
          CurrentState = Waiting2Write;
        }
      }
      break;
    case Waiting2Write :
      if (ThisEvent.EventType == ES_LCD_PUTCHAR ){
        // Write Data
        LCD_WriteData8(ThisEvent.EventParam);
        // start the inter-character timer
        ES_ShortTimerStart(TIMER_B, INTER_CHAR_DELAY);
        // move to the PausingBetweenWrites state
        CurrentState = PausingBetweenWrites;
      }
      break;
    case PausingBetweenWrites :
      // if this was a short timeout event, 
      if (ThisEvent.EventType == ES_SHORT_TIMEOUT ){
        // then recall any defered events and move to the Waiting2Write state
        ES_RecallEvents(MyPriority, DeferralQueue);
        CurrentState = Waiting2Write;
      }
      // if this was an LCD_Putchar, 
      else if (ThisEvent.EventType == ES_LCD_PUTCHAR) {
        // then defer any new characters that arrive while pausing between LCD writes
        ES_DeferEvent(DeferralQueue, ThisEvent);
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

