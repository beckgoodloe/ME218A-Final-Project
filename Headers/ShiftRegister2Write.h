// Header for Shift Register Write Operations

#ifndef SHIFTREGISTER2WRITE_H
#define SHIFTREGISTER2WRITE_H

#include <stdint.h>
#include <stdbool.h>

void SR2_Init(void);
uint16_t SR2_GetCurrentRegister(void);
void SR2_Write(uint16_t NewValue);

#endif
