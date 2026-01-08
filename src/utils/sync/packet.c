#include "packet.h"
#include "sync.h"


void process_sup_frame(sup_frame_t* frame)
{
    switch (frame->id)
    {
        case BOOT_REQUEST_UPDATE:
            switch_to_bootloader(); 
            break;
        
        default:
            break;
    }
}
