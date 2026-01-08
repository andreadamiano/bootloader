#include "sync.h"

volatile uint32_t flag = 0;


void set_firmware_update_flag()
{
    flag = FW_UPDATE_REQUEST; 
}

void switch_to_bootloader()
{
    set_firmware_update_flag(); 
    wdt_enable(WDTO_15MS); //software reset 
}