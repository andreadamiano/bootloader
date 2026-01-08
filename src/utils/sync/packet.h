#ifndef PACKET_H
#define PACKET_H

#include "../protocol/sup.h"

typedef enum boot_packet
{
    BOOT_REQUEST_UPDATE,
    BOOT_DATA,
    BOOT_MAX
}boot_packet;

void process_sup_frame(sup_frame_t* frame);

#endif