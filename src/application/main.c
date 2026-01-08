#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h> 
#include "utils/protocol/sup.h"
#include "utils/USART/USART.h"
#include <stdio.h>
#include "utils/sync/packet.h"
#include <avr/wdt.h>

/* persistent reset counter in .noinit to detect repeated resets */
volatile uint8_t reset_count __attribute__((section(".noinit")));

ISR(USART_RX_vect)
{
    const uint8_t byte = UDR0; // get received byte 
    sup_handle_rx_byte(byte); 

    //check if a complete sup frame was received 
    sup_rx_frame_state_t* current_state = sup_get_rx_state(); 
    if ((current_state != NULL) && (current_state->parsing_result == SUP_RESULT_SUCCESS))
    {
        process_sup_frame(&current_state->frame); 
    }
}


int main()
{
    wdt_disable(); 
    
    //initialize usart and sup protocol 
    sup_rx_frame_state_t current_state; 
    initUSART();
    sup_init(&current_state); 

    //enable interrupts
    sei(); 

    DDRB = 1;  

    while (true)
    {
        PORTB ^= 1; 
        _delay_ms(100); 
    }
    
}