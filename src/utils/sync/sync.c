#include "sync.h"

volatile uint32_t flag __attribute__ ((section(".bootloader-sync"))); 