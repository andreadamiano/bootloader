#include "packet.h"
#include "sync.h"


void processSupFrame(sup_frame_t* frame)
{
    switch (frame->id)
    {
        case BOOT_REQUEST_UPDATE:
            switchToBootloader(); 
            break;
        
        default:
            break;
    }
}
