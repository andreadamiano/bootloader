#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h> 
#include "utils/protocol/sup.h"
#include "utils/USART/USART.h"

ISR(USART_RX_vect)
{
    const uint8_t byte = UDR0; // get received byte 
    sup_handle_rx_byte(byte); 
}


int main()
{
    //initialize usart and sup protocol 
    sup_rx_frame_state_t current_state; 
    initUSART();
    sup_init(&current_state); 


    DDRB = 1;  

    while (true)
    {
        PORTB ^= 1; 
        _delay_ms(100); 
    }
    
}