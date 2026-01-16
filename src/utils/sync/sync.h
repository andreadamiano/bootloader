#ifndef SYNC_H
#define SYNC_H

#include <stdlib.h>
#include <stdint.h>
#include <avr/wdt.h>
#include <util/delay.h>


#define FW_UPDATE_REQUEST (0xDEADBEEFUL)


typedef enum fw_parsing_state{
    FW_STATE_IDLE, 
    FW_STATE_READY,    
    FW_STATE_RECEIVING, 
    FW_STATE_FINISHED,  
    FW_STATE_ERROR      
}fw_parsing_state; 

extern volatile uint32_t flag;


void setFirmwareUpdateFlag(); 
void switchToBootloader(); 

#endif

