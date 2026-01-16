#include "sync.h"
#include <avr/interrupt.h>
#include "../USART/USART.h"
#include <stdio.h>

volatile uint32_t flag __attribute__((section(".noinit")));

void setFirmwareUpdateFlag(void)
{
    flag = FW_UPDATE_REQUEST;
}


void switchToBootloader()
{
    cli(); //disable interrupt 

    setFirmwareUpdateFlag(); 

    char debug[30];
    sprintf(debug, "Flag set to: 0x%08lX\n", flag);
    printString(debug);

    // char debug2[30] = "received boot signal\n";
    // printString(debug2); 

    _delay_ms(1000); 
    wdt_enable(WDTO_250MS);
    while (1)
    {
    
    }
}