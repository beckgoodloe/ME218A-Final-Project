/****************************************************************************
 Module
   Interaction.c

 Revision
   0.0.1

 Description
   This module acts as the low level interface to all YAM outputs.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/09/19 19:55 tnc     first pass
 
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

#include "ADMulti.h"
#include "PWM16Tiva.h"
#include "ShiftRegisterWrite.h"
#include "ShiftRegister2Write.h"
#include "ES_Configure.h"
#include "AlienSM.h"
#include "Interaction.h"
#include "SoundboardSM.h"


// readability defines
#define DIAL_ADC_BIT  0
#define FAN_ADC_BIT  1
#define NUM_PWM_OUT   4
#define NUM_ANALOG_IN 2
#define SERVO_PERIOD  25000 // * 0.8us
#define FUEL_SERVO_GROUP 0
#define TIME_SERVO_GROUP 1
#define TOT_SERVO_GROUP 2
#define FIRE_CHANNEL 3
#define ONE_MS        1250
#define FUEL_ADC_MAX       3800
#define TIME_MAX      50000
#define ROTATION_LIMITER 200
#define ALIEN_LED_COUNT 4
#define HEALTH_ALL_HI (BIT0HI | BIT1HI | BIT2HI | BIT3HI)
#define HEALTH_ALL_LO (BIT0LO & BIT1LO & BIT2LO & BIT3LO)
#define FUEL_MAX (BIT3HI | BIT4HI | BIT5HI | BIT6HI | BIT7HI)
#define FUEL_ZEROS (BIT3LO & BIT4LO & BIT5LO & BIT6LO & BIT7LO)
#define FIRESTARTER_LED_HI BIT1HI
#define FIRESTARTER_LED_LO BIT1LO
#define AIR_LED_HI BIT2HI
#define AIR_LED_LO BIT2LO

static uint8_t FirePWM;
static uint8_t LocalRegisterImage=0;
static uint32_t AnalogIn[4];

void InitInteraction(void){
  printf("initializing Interaction\r\n");
  ADC_MultiInit(NUM_ANALOG_IN);
  PWM_TIVA_Init(NUM_PWM_OUT);
  PWM_TIVA_SetPeriod(SERVO_PERIOD, 0);
  PWM_TIVA_SetPeriod(SERVO_PERIOD, 1);
  // Set PB0, PB1, PB2 to Data, Sclk, Rclk outputs
  SR_Init();
  SR2_Init();
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R2) != SYSCTL_PRGPIO_R2) 
	{ 
	} 

	// Set PB3 (TOT IR), PB5(firestarter), PE2-4(alienbutt1-3), to be digital
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (BIT5HI | BIT6HI | BIT7HI);
  HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= (BIT4HI | BIT5HI);
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT3HI);
  HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= (BIT2HI | BIT3HI | BIT4HI);
  
  // Set PB3-4, PE2-4 to input
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) &= (BIT5LO);
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) &= (BIT3LO);
  HWREG(GPIO_PORTE_BASE+GPIO_O_DIR) &= (BIT2LO & BIT3LO & BIT4LO);
  // Enable internal pullup on PB4, PE2-4 (buttons)
  HWREG(GPIO_PORTA_BASE+GPIO_O_PUR) |= (BIT5HI);
  HWREG(GPIO_PORTE_BASE+GPIO_O_PUR) |= (BIT2HI | BIT3HI | BIT4HI);
  
  // Set PA6 as output (sound 1 - success)
  // Set PA7 as output (sound 2 - fail)
  HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= (BIT6HI | BIT7HI);
//  // Set PC4 as output (sound 3 - fire)
  HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) |= (BIT4HI | BIT5HI);
  
  HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) |= (BIT6HI | BIT7HI);
  HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) |= (BIT4HI | BIT5HI);

  TurnOffFuelLEDs();
}

void BoostFire(void) {
  FirePWM -= 10;
  PWM_TIVA_SetDuty(FirePWM, FIRE_CHANNEL);
  HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) &= (BIT5LO);
  ES_Event_t Event2Post;
  Event2Post.EventType = Fire_Sound;
  PostSoundboardSM(Event2Post);
  printf("BOOST fire to %" PRIu8 "\n\r", FirePWM);
}

void DimFire(void) {
  FirePWM += 10;
  PWM_TIVA_SetDuty(FirePWM, FIRE_CHANNEL);
  HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) |= (BIT5HI);
  printf("DIM fire to %" PRIu8 "\n\r", FirePWM);
}

void MaxFire(void) {
  FirePWM = 0;
  PWM_TIVA_SetDuty(FirePWM, FIRE_CHANNEL);
  HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) &= (BIT5LO);
  printf("MAX Fire Intensity\n\r");
}

void FireOff(void)  {
  FirePWM = 100;
  PWM_TIVA_SetDuty(FirePWM, FIRE_CHANNEL);
  HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) |= (BIT5HI);
  printf("Fire off\n\r");
}

void MoveFuelServo(uint16_t DialReading) {
  float ScaledReading = (float)(ONE_MS+700)*((float)DialReading / (float)(FUEL_ADC_MAX));
  uint16_t NewServoPosition = (ScaledReading + (ONE_MS-300));
  //printf("MOVE fuel to position %" PRIu16 "\n\r",NewServoPosition);
  PWM_TIVA_SetPulseWidth(NewServoPosition, FUEL_SERVO_GROUP);
}

void SetFuelLEDs(uint8_t CorrectPosition) {
  printf("Set fuel LEDs to position %" PRIu8 "\n\r", CorrectPosition);
  uint16_t NewValue;
  if (CorrectPosition == 5) {
    NewValue = SR2_GetCurrentRegister() & FUEL_ZEROS;
    NewValue |= (BIT7HI);
  } else if (CorrectPosition == 4) {
    NewValue = SR2_GetCurrentRegister() & FUEL_ZEROS;
    NewValue |= (BIT6HI);
  } else if (CorrectPosition == 3) {
    NewValue = SR2_GetCurrentRegister() & FUEL_ZEROS;
    NewValue |= (BIT5HI);
  } else if (CorrectPosition == 2) {
    NewValue = SR2_GetCurrentRegister() & FUEL_ZEROS;
    NewValue |= (BIT4HI);
  } else if (CorrectPosition == 1) {
    NewValue = SR2_GetCurrentRegister() & FUEL_ZEROS;
    NewValue = (BIT3HI);
  }
  SR2_Write(NewValue);
}

void TurnOffFuelLEDs(void) {
  SR2_Write(SR2_GetCurrentRegister() & FUEL_ZEROS);
}
  
void MoveKitchenTimer(uint8_t TimeLeft) {
  float ScaledReading = (float)ONE_MS*1.1*(((float)TimeLeft *(float)1000) / (float)(TIME_MAX));
  uint16_t NewPosition = (ScaledReading + ONE_MS);
  //printf("MOVE kitchen to position %" PRIu16 "\n\r",NewPosition);
  PWM_TIVA_SetPulseWidth(NewPosition, TIME_SERVO_GROUP);
}

void OpenTOTServo(void) {
  PWM_TIVA_SetPulseWidth(ONE_MS + ONE_MS, TOT_SERVO_GROUP);
}

void CloseTOTServo(void) {
  PWM_TIVA_SetPulseWidth(ONE_MS - 300, TOT_SERVO_GROUP);
}

void TurnOnFireStarterLEDs(void) {
  uint16_t NewValue;
  NewValue = SR2_GetCurrentRegister() | FIRESTARTER_LED_HI;
  SR2_Write(NewValue);
  printf("turning ON FireStarter LEDs\n\r");
}

void TurnOffFireStarterLEDs(void) {
  uint16_t NewValue;
  NewValue = SR2_GetCurrentRegister() & FIRESTARTER_LED_LO;
  SR2_Write(NewValue);
  printf("turning OFF FireStarter LEDs\n\r");
}

void TurnOnAirLEDs(void) {
  uint16_t NewValue;
  NewValue = SR2_GetCurrentRegister() | AIR_LED_HI;
  SR2_Write(NewValue);
  printf("turning ON Mic LEDs\n\r");
}

void TurnOffAirLEDs(void) {
  uint16_t NewValue;
  NewValue = SR2_GetCurrentRegister() & AIR_LED_LO;
  SR2_Write(NewValue);
  printf("turning OFF Mic LEDs\n\r");
}

void ActivateAlien(void) {
  printf("Alien Activated\n\r");
  uint8_t CurrentRegister = SR_GetCurrentRegister();
  SR_Write(CurrentRegister | BIT4HI | HEALTH_ALL_HI);
}

void TurnOnAlien1LED(void) {
  printf("turning on alien 1 LED\n\r");
  uint8_t CurrentRegister = SR_GetCurrentRegister();
  SR_Write(CurrentRegister | BIT7HI);
}

void TurnOnAlien2LED(void) {
  printf("turning on alien 2 LED\n\r");
  uint8_t CurrentRegister = SR_GetCurrentRegister();
  SR_Write(CurrentRegister | BIT6HI);
}

void TurnOnAlien3LED(void) {
  printf("turning on alien 3 LED\n\r");
  uint8_t CurrentRegister = SR_GetCurrentRegister();
  SR_Write(CurrentRegister | BIT5HI);
}

void TurnOffAlien1LED(void) {
  printf("turning off alien 1 LED\n\r");
  uint8_t CurrentRegister = SR_GetCurrentRegister();
  SR_Write(CurrentRegister & BIT7LO);
}

void TurnOffAlien2LED(void) {
  printf("turning off alien 2 LED\n\r");
  uint8_t CurrentRegister = SR_GetCurrentRegister();
  SR_Write(CurrentRegister & BIT6LO);
}

void TurnOffAlien3LED(void) {
  uint8_t CurrentRegister = SR_GetCurrentRegister();
  SR_Write(CurrentRegister & BIT5LO);
}

void DimAlienLEDs(void) {
  printf("DIM Alien LEDs -10\n\r");
  uint8_t CurrentRegister = SR_GetCurrentRegister();
  uint8_t NextLEDs;
  if (GetAlienHitCount() == 1) {
    NextLEDs = CurrentRegister & BIT3LO;
    SR_Write(NextLEDs | BIT0HI | BIT1HI | BIT2HI);
  } else if (GetAlienHitCount() == 2) {
    NextLEDs = CurrentRegister & BIT3LO & BIT2LO;
    SR_Write(NextLEDs | BIT0HI | BIT1HI);
  } else if (GetAlienHitCount() == 3) {
    NextLEDs = CurrentRegister & BIT3LO & BIT2LO & BIT1LO;
    SR_Write(NextLEDs | BIT0HI);
  } else if (GetAlienHitCount() == 4) {
    SR_Write(CurrentRegister & HEALTH_ALL_LO);
  }
}

void DeactivateAlien(void) {
  printf("turning OFF Alien LEDs\n\r");
  SR_Write(0x00);
  printf("alien deactivated\n\r");
}

uint8_t CheckTOT(void) {
  return (HWREG(GPIO_PORTB_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT3HI);
}

uint8_t GetFirestarterButton(void) {
  return (HWREG(GPIO_PORTA_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT5HI);
}

uint16_t GetMicIntensity(void) {
  ADC_MultiRead(AnalogIn);
  return AnalogIn[FAN_ADC_BIT];
}

uint16_t GetDialPosition(void) {
  ADC_MultiRead(AnalogIn);
  //printf("getting Dial Position: %u \n\r", AnalogIn[DIAL_ADC_BIT]);
  return AnalogIn[DIAL_ADC_BIT];
}

uint8_t GetAlien1Button(void) {
  return (HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT2HI);
}

uint8_t GetAlien2Button(void) {
  return (HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT3HI);
}

uint8_t GetAlien3Button(void) {
  return (HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT4HI);
}