#ifndef SYNC_H
#define SYNC_H

#include <stdlib.h>
#include <stdint.h>
#include <avr/wdt.h>
#include <util/delay.h>


#define FW_UPDATE_REQUEST (0xDEADBEEFUL)

extern volatile uint32_t flag;


void setFirmwareUpdateFlag(); 
void switchToBootloader(); 

#endif

