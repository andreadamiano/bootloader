#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif


// #include <avr/interrupt.h>
// #include <avr/io.h>
// #include <util/delay.h>
// #include <stdlib.h>
// #include <avr/wdt.h>

// ISR(USART_RX_vect)
// {
//     uint8_t receivedByte = UDR0;  
// }
 

// int main ()
// {
//     MCUSR &= ~(1 << WDRF); 
//     wdt_disable();          
//     DDRB |= 0; 
//     while (1)
//     {
//         PORTB = 1; 
//         // _delay_ms(100); 
//     }
    
// }
 
  

#include <avr/io.h>
#include <avr/wdt.h>

int main(void) {
    MCUSR = 0;  // Clear ALL reset flags
    wdt_disable();
    
    // Set ALL PORTB pins as outputs
    DDRB = 0xFF;
    
    // Light up ALL PORTB pins
    PORTB = 0xFF;
    
    while(1);
}