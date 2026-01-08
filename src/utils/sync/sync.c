#include "sync.h"
#include <avr/interrupt.h>


volatile uint32_t flag __attribute__((section(".noinit")));


void set_firmware_update_flag()
{
    flag = FW_UPDATE_REQUEST;
}

void switch_to_bootloader()
{
    cli(); //disable interrupt 
    set_firmware_update_flag();
    wdt_enable(WDTO_1S); //software reset 
    while (1)
    {
    
    }
}