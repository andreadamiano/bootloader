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
#include <avr/eeprom.h>
#include <avr/boot.h>
#include <util/delay.h>

#define APP_START_ADDR (0x0000) 
#define MAX_APPLICATION_SIZE (0x700)  //28 KB
#define WORD_SIZE_BYTES (2U)  //a word correspond to 2 bytes 
#define FLASH_EMPTY_WORD (0xFFFFU) 


// extern volatile uint32_t flag; 
// extern uint32_t flag; 
fw_parsing_state fw_state = FW_STATE_IDLE;
static uint16_t          fw_expected_size  = 0;      
static uint16_t          fw_received_bytes = 0;      
static uint16_t          fw_write_address  = 0;      
static uint8_t           fw_page_buffer[SPM_PAGESIZE]; 
static uint16_t          fw_page_buffer_index = 0;     


static bool write_flash_page(const uint16_t page_addr, const uint8_t* page_data)
{
    //check page alignment 
    if ((page_addr % SPM_PAGESIZE) != 0)
        return false; 


    //disable interrupts
    const uint8_t sreg = SREG; 
    cli(); 

    //wait for any eeprom pending operations 
    eeprom_busy_wait(); 

    uint16_t word_data; 
    uint8_t low_byte; 
    uint8_t high_byte; 
    for (int offset =0; offset < SPM_PAGESIZE; offset += WORD_SIZE_BYTES)
    {
        word_data = FLASH_EMPTY_WORD; 

        if (page_data != NULL)
        {
            low_byte = page_data[offset]; 
            high_byte = offset + 1 < MAX_APPLICATION_SIZE ? page_data[++offset] : 0xFF; 
            word_data = ((uint16_t)high_byte << 8) | (uint16_t) low_byte;  
        }

        //load word into temporary flash buffer
        boot_page_fill(page_addr+offset, word_data); 
    }

    //erase flash page 
    boot_page_erase(page_addr);
    boot_spm_busy_wait(); 
    
    //write page buffer to flash 
    boot_page_write(page_addr);
    boot_spm_busy_wait(); 

    //re-enable read write section access
    boot_rww_enable();

    //restore interrupt state 
    SREG = sreg; 
        
    return true; 
}


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

void processSupFrame(sup_frame_t* frame)
{
    if(frame->id != SUP_ID_DATA)
    {
        sup_send_nack(frame->id,  (const uint8_t*)&fw_state); //send the state back so the sender can implement some error handling logic 
        return; 
    }

    switch (fw_state)
    {
        case FW_STATE_IDLE:
        case FW_STATE_FINISHED:
        case FW_STATE_ERROR:
            // Not expecting data in these states
            sup_send_nack(frame->id, (const uint8_t*)&fw_state);
            break;

        case FW_STATE_READY:
            //waiting for the firware size (express as 2 bytes little endian)
            if (frame->payload_size == 0)
            {
                fw_state = FW_STATE_ERROR; 
                sup_send_nack(frame->id, (const uint8_t*)&fw_state); 
            }
            
            fw_expected_size = (uint16_t) frame->payload[0] | ((uint16_t) frame->payload[1] << 8); //big endian 

            if ((fw_expected_size == 0 ) || (fw_expected_size > MAX_APPLICATION_SIZE))
            {
                sup_send_nack(frame->id, (const uint8_t*) &fw_state); 
                fw_state = FW_STATE_ERROR; 
                break;
            }

            //prepare firmware reception
            fw_received_bytes    = 0;
            fw_write_address     = APP_START_ADDR;
            fw_page_buffer_index = 0;
            memset(fw_page_buffer, 0xFF, sizeof(fw_page_buffer)); // Erased state

            fw_state = FW_STATE_RECEIVING; 
            sup_send_ack(frame->id, (const uint8_t*)&fw_state); 

            char debug [30];  
            sprintf(debug, "boot size received %d\n", fw_expected_size); 
            printString(debug); 
            break;

        case FW_STATE_RECEIVING:

            //write firmware data payload to flash memory
            for (uint8_t i = 0; i < frame->payload_size ; ++i)
            {
                if (fw_received_bytes > fw_expected_size)
                {
                    fw_state = FW_STATE_ERROR; 
                    sup_send_nack(frame->id, (const uint8_t*) &fw_state); 
                    return;
                }

                //add byte to page buffer
                fw_page_buffer[fw_page_buffer_index++] = frame->payload[i];
                fw_received_bytes++;

                //check if we filled page buffer or we uploaded all the firmware 
                if (fw_page_buffer_index >= SPM_PAGESIZE || fw_received_bytes >= fw_expected_size)
                {
                    //write page buffer 
                    if (!write_flash_page(fw_write_address, fw_page_buffer))
                    {
                        fw_state = FW_STATE_ERROR;
                        sup_send_nack(frame->id, (const uint8_t*)&fw_state);
                        return;
                    }

                    fw_write_address += SPM_PAGESIZE; 
                    fw_page_buffer_index = 0; 
                    memset(fw_page_buffer, 0xff, sizeof(fw_page_buffer)); 
                }
            }

            sup_send_ack(frame->id, (const uint8_t*)&fw_state ); 

            //check if we finished to upload all the firmware 
            if (fw_received_bytes == fw_expected_size)
            {
                fw_state = FW_STATE_FINISHED; 
                sup_send_frame(SUP_ID_ACK, NULL, 0); 
            }
            break;
        
        default:
            break;
    }
}

int main ()
{
    uint8_t mcusr_val = MCUSR;
    (void)mcusr_val;
    MCUSR = 0; //the MCU status register must be cleared otherwise the wdt cannot be disabled 

    wdt_disable(); 
    
    //initialize USART to send debug messages 
    initUSART();
    
    char string[30] = "inside bootloader\n"; 
    printString(string); 

    uint32_t flag_value = eeprom_read_dword((uint32_t*)FLAG_EEPROM_ADDR);

    //slow blink to signal entering the bootloader section
    DDRB |= 1; 
    for (int i = 0; i < 4; ++i)
    {
        PORTB ^= 1; 
        _delay_ms(1000); 
    }

    char debug[50];
    sprintf(debug, "Flag value: 0x%08lX\n", flag_value);
    printString(debug);
    usart_flush(); 

    if (flag_value == FW_UPDATE_REQUEST)
    {
        //reset flag 
        eeprom_write_dword((uint32_t*)FLAG_EEPROM_ADDR, 0xFFFFFFFF);

        sup_rx_frame_state_t current_state; 
        sup_init(&current_state); 
        fw_state = FW_STATE_READY; 
        
        strcpy(string, "updating firmware"); 
        printString(string); 

        //start polling USART untill the firmware is updated 
        while (1)
        {
            uint8_t byte = receiveByte(); 
            sup_handle_rx_byte(byte); 

            sup_rx_frame_state_t* current_state = sup_get_rx_state(); 
            if ((current_state != NULL) && (current_state->parsing_result == SUP_RESULT_SUCCESS))
            {
                processSupFrame(&current_state->frame); 

                if (fw_state == FW_STATE_FINISHED)
                    break;

                if (fw_state == FW_STATE_ERROR)
                {
                    strcpy(string, "error fw retrieve"); //debug message
                    printString(string); 
                }
            }
        }
    
        
    }
    jump_to_application(); 
    return 0; 
        
}