#include "sync.h"
#include <avr/interrupt.h>
#include "../USART/USART.h"

volatile uint32_t flag __attribute__((section(".noinit")));

void set_firmware_update_flag(void)
{
    flag = FW_UPDATE_REQUEST;
}


void switch_to_bootloader()
{
    cli(); //disable interrupt 
    // set_firmware_update_flag(); 

    char debug[30] = "received boot signal\n";
    _delay_ms(1000); 
    printString(debug); 
    wdt_enable(WDTO_250MS);
    while (1)
    {
    
    }
}