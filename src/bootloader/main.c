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
    wdt_disable(); 

    // move interrupt vector to application section 
    MCUCR = _BV(IVCE); 
    MCUCR = 0;  
   
    cli(); 
    UCSR0B = 0; // Disable UART

    __asm__ __volatile__("clr r1\n\t"               // Clear register r1 (zero register)
                         "jmp %0"                   // Jump to application
                         :                          // No output operands
                         : "i"(APP_START_ADDR) // Input: application start address
    );

    __builtin_unreachable(); 
}


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