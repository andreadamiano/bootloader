#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include "utils/USART/USART.h"
#include "utils/protocol/sup.h"
#include "utils/sync/sync.h"
#include <string.h>
#include <stdio.h>

#define APP_START_ADDR (0x0000) 
extern volatile uint32_t flag; 


static void __attribute__((noreturn)) jump_to_application(void)
{
    wdt_disable();

    //move interrupt vectors back to application section
    MCUCR = _BV(IVCE); //enable interrupt vector change
    MCUCR = 0;         //move vectors to application section

    cli();

   //disable UART
    UCSR0B = 0;

    //inline assembly
    __asm__ __volatile__("clr r1\n\t"               //clear register 1
                         "jmp %0"                   //jump to application (%0 is a placeholder)
                         :                          //output operands
                         : "i"(APP_START_ADDR) //input operands
    );

    __builtin_unreachable();
}

int main ()
{
    uint8_t mcusr_val = MCUSR;
    MCUSR = 0; //the MCU status register must be cleared otherwise the wdt cannot be disabled 

    wdt_disable(); 
    
    //initialize USART to send debug messages 
    initUSART();

    if (mcusr_val & _BV(WDRF)) {
        char debug[30] = "WDT reset detected\n";
        printString(debug);
    }
    
    char string[30] = "inside bootloader\n"; 
    printString(string); 

    //slow blink to signal entering the bootloader section
    DDRB |= 1; 
    for (int i = 0; i < 4; ++i)
    {
        PORTB ^= 1; 
        _delay_ms(1000); 
    }

    char debug[50];
    sprintf(debug, "Flag value: 0x%08lX\n", flag);
    printString(debug);



    if (flag == FW_UPDATE_REQUEST)
    {
        sup_rx_frame_state_t current_state; 
        sup_init(&current_state); 
        
        strcpy(string, "updating firmware"); 
        printString(string); 


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