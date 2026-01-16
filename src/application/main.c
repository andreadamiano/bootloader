#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h> 
#include "utils/protocol/sup.h"
#include "utils/USART/USART.h"
#include <stdio.h>
#include "utils/sync/packet.h"
#include <avr/wdt.h>
#include <util/atomic.h>
#include <string.h>

volatile bool receiveNewFrame = false;  

ISR(USART_RX_vect)
{
    const uint8_t byte = UDR0; // save received byte via USART
    sup_handle_rx_byte(byte); 

    //check if a complete sup frame was received 
    sup_rx_frame_state_t* current_state = sup_get_rx_state(); 
    if ((current_state != NULL) && (current_state->parsing_result == SUP_RESULT_SUCCESS))
    {
        receiveNewFrame = true; 
    }
}

void copySupFrame(sup_frame_t* in, sup_frame_t* out)
{
    uint8_t sreg = SREG;  
    cli(); //disable all interrupts

    out->id = in->id; 
    out->payload_size = in->payload_size; 

    if (in->payload_size >0 && in->payload_size < SUP_MAX_PAYLOAD_SIZE)
        memcpy(out, in, in->payload_size); 


    SREG = sreg; 
}

void blinkLed()
{
    PORTB ^= 1; 
    _delay_ms(100); 
}


int main()
{
    wdt_disable(); 
    
    //initialize usart and sup protocol 
    sup_rx_frame_state_t current_state; 
    initUSART();
    sup_init(&current_state); 
    
    char string[30] = "inside application\n"; 
    printString(string); 
    
    //enable interrupts
    sei(); 

    DDRB = 1;  

    while (true)
    {
        if (receiveNewFrame)
        {
            receiveNewFrame = false; 
            sup_frame_t copy; 
            copySupFrame(&current_state.frame, &copy); 
            process_sup_frame(&copy);
        }
        blinkLed(); 
    }
    
}