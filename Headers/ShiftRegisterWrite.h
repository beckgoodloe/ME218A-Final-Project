// Header for Shift Register Write Operations

#ifndef SHIFTREGISTERWRITE_H
#define SHIFTREGISTERWRITE_H

#include <stdint.h>
#include <stdbool.h>

void SR_Init(void);
uint8_t SR_GetCurrentRegister(void);
void SR_Write(uint8_t NewValue);

#endif
