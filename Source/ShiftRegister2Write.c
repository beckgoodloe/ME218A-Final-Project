/****************************************************************************
 Module
   ShiftRegisterWrite.c

 Revision
   1.0.1

 Description
   This module acts as the low level interface to a write only shift register.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/11/15 19:55 jec     first pass
 
****************************************************************************/
// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

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

// readability defines
#define DATA GPIO_PIN_2
#define DATA_HI BIT2HI
#define DATA_LO BIT2LO

#define SCLK GPIO_PIN_3
#define SCLK_HI BIT3HI
#define SCLK_LO BIT3LO

#define RCLK GPIO_PIN_4
#define RCLK_LO BIT4LO
#define RCLK_HI BIT4HI


// an image of the last 16 bits written to the shift register
static uint16_t LocalRegisterImage=0;

void SR2_Init(void){
  
  // Enable Port A, wait for Port A to be enabled
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0) 
	{ 
	} 
 
	// Set bit 0, 1, 2 of Port A to be digital
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (BIT2HI | BIT3HI | BIT4HI);

	// Set bit 0, 1, 2 of Port B to be an output
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= (BIT2HI | BIT3HI | BIT4HI);
  
  // start with the data & sclk lines low and the RCLK line high
  HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) |= (BIT4HI);
  HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= (BIT2LO & BIT3LO);

}

// Get current state of bits in shift register
uint16_t SR2_GetCurrentRegister(void){
  return LocalRegisterImage;
}

// Write data to outputs of shift register
void SR2_Write(uint16_t NewValue){
  

  uint16_t BitCounter;
  LocalRegisterImage = NewValue; // save a local copy
  uint16_t LocalTemp = NewValue;

  // lower the register clock
  HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= RCLK_LO;

  // shift out the data while pulsing the serial clock 
  for (BitCounter=0; BitCounter<16; BitCounter++)
    {  
      // Isolate the MSB of NewValue, put it into the LSB position and output to port
      if (LocalTemp & BIT15HI)
      {
        HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) |= DATA_HI;
      }
      else {
        HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= DATA_LO;
      }
      // raise SCLK
      HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) |= SCLK_HI;
      // lower SCLK
      HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= SCLK_LO;
      
      // shift bits towards MSB
      LocalTemp = LocalTemp << 1;

    }
  
    // raise the register clock to latch the new data
    HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) |= RCLK_HI; 


}
