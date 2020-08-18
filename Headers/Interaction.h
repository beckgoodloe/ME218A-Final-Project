#ifndef Interaction_H
#define Interaction_H

// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>

void InitInteraction(void);

void BoostFire(void);
void MaxFire(void);
void DimFire(void);
void FireOff(void);
void MoveFuelServo(uint16_t DialReading);
void SetFuelLEDs(uint8_t CorrectPosition);
void TurnOffFuelLEDs(void);
void MoveKitchenTimer(uint8_t TimeLeft);
void OpenTOTServo(void);
void CloseTOTServo(void);
void TurnOnFireStarterLEDs(void);
void TurnOffFireStarterLEDs(void);
void TurnOnAirLEDs(void);
void TurnOffAirLEDs(void);
void ActivateAlien(void);
void DimAlienLEDs(void);
void TurnOnAlien1LED(void);
void TurnOnAlien2LED(void);
void TurnOnAlien3LED(void);
void TurnOffAlien1LED(void);
void TurnOffAlien2LED(void);
void TurnOffAlien3LED(void);
void DeactivateAlien(void);

uint8_t GetFirestarterButton(void);
uint16_t GetMicIntensity(void);
uint16_t GetDialPosition(void);
uint8_t GetAlien1Button(void);
uint8_t GetAlien2Button(void);
uint8_t GetAlien3Button(void);

uint8_t CheckTOT(void);


#endif //Interaction_H
