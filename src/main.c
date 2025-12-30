#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include "USART/USART.h"

ISR(USART_RX_vect)
{
    uint8_t receivedByte = UDR0;  
}
 

int main ()
{

    // select the interrupt vector of the bootloader section
    MCUCR = (1<<IVCE);
    MCUCR = (1<<IVSEL);

    wdt_disable(); 

    //init USART 
    initUSART(); 


    MCUSR &= ~(1 << WDRF);         
    DDRB |= (1 << PB0); 
    while (1)
    {
        PORTB ^= (1 << PB0); 
        _delay_ms(100); 
    }
    
}