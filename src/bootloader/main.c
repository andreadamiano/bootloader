#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include "utils/USART/USART.h"
#include "utils/protocol/sup.h"
#include "utils/sync/sync.h"

#define APP_START_ADDR (0x0000) 
extern volatile uint32_t flag; 

void process_sup_frame(const sup_frame_t* frame)
{

}

static void  __attribute__((noreturn)) jump_to_application()
{

    // Prefer a clean hardware reset so the application runs through
    // its normal startup (crt0) rather than trying to emulate reset
    // by jumping. This requires the BOOTRST fuse to be cleared
    // (so reset goes to application). Trigger a short WDT reset.

    wdt_disable();
    cli();
    UCSR0B = 0; // Disable UART

    //move interrupt vector pointer back to the application section
    MCUCR = _BV(IVCE); // Enable interrupt vector change
    MCUCR = 0;  

    // Enable WDT for shortest timeout, then wait for reset
    wdt_enable(WDTO_15MS);
    for (;;) { /* wait for WDT reset */ }

    __builtin_unreachable();
}


// static void  __attribute__((noreturn)) jump_to_application()
// {
//     // Disable interrupts immediately
//     cli();

//     // Disable UART/other peripherals
//     UCSR0B = 0;
//     PRR = 0xFF; // power down peripherals (timers, TWI, ADC, USART, etc)

//     // Ensure interrupt vectors point to application (atomic IVCE/IVSEL sequence)
//     MCUCR = _BV(IVCE);
//     MCUCR = 0; // IVSEL = 0 -> vectors at 0x0000 (application)

//     // Set stack pointer to top of application RAM
//     // RAMEND is defined in <avr/io.h>
//     uint16_t sp = RAMEND;
//     SPL = (uint8_t)(sp & 0xFF);
//     SPH = (uint8_t)(sp >> 8);

//     // Jump to application reset vector (absolute jump)
//     asm volatile ("jmp %0" :: "i" (APP_START_ADDR));

//     __builtin_unreachable();
// }


int main ()
{
     
    // debug (enter bootloader section)
    DDRB = 1; 
    for (int i = 0; i < 4; ++i)
    {
        PORTB ^= 1; 
        _delay_ms(1000); 
    }


    //disable watch dog timer 
    wdt_disable(); 


    if (flag == FW_UPDATE_REQUEST)
    {
        //init USART 
        sup_rx_frame_state_t current_state; 
        initUSART();
        sup_init(&current_state); 


        //start polling USART untill the firmware is updated 
        while (current_state.parsing_result != SUP_RESULT_SUCCESS)
        {   
            uint8_t byte = receiveByte(); 
            sup_handle_rx_byte(byte); 
        }
        
    }
    jump_to_application(); 
    return 0; 
        
}