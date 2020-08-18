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

#include "FuelService.h"
/*----------------------------- Module Defines ----------------------------*/
#define SCALING_FACTOR 0.5 //arbitrary value, for now


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
void MoveFuelServo(uint32_t DialReading);


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static FuelServiceState_t CurrentState;
static uint16_t CurrentPosition;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitFuelService

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
bool InitFuelService(uint8_t Priority)
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
     PostFuelService

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
bool PostFuelService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunFuelService

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
ES_Event_t RunFuelService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  /********************************************
   in here you write your service code
   *******************************************/
  // Two States for this service: FuelNeeded and FuelOK
  switch(CurrentState){
    case FuelNeeded:
      //If ThisEvent.EventType is a pot change event:
        // if current position is in the right range:
          // set current position to pot_value * scaling factor
          // update servo position to current position
          printf("Enough Fuel");
          CurrentState = FuelOK;
        // else
          // stop fuelfailtimer
          // set currentposition to pot_value* scaling factor
          // update servo position to current position
          // init fuel event timer
        //end ifelse
      //endif
      break;
    case FuelOK:
      //If ThisEvent.EventType is a pot change event:
        //if current position is NOT in the right range
          // set currentposition to pot_value *scalingfactor
          // update servo position to currentposition
          // post firedim event to maingame sm
          // init fuelfailtimer
          printf("Fuel Needed");
          CurrentState = FuelNeeded;
        //else
          // set currentposition to pot_value*scaling factor
          // update servo position to current position
          // MIGHT NEED TO SET CORRECT RANGE??? Lil confused from state diagram
        //end ifelse
      //endif
      
      //If ThisEvent.EventType is the FuelEventTimer timing out
        // Post FireDim Event to MainGameSM
        // Init FuelFailTimer
        // Set new CorrectPosition
        // Light Fuel LEDS to Correct Position
        printf("Need Fuel again");
        CurrentState = FuelNeeded;
      //endif
      break;
  }
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
void MoveFuelServo(uint32_t DialReading){

}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

