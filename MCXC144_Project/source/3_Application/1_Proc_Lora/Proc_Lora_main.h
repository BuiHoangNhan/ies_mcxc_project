#ifndef PROC_LORA_MAIN_H
#define PROC_LORA_MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <2_Middle/1_Lora/Middle_Lora.h>

#define MAX_NBR_PAINID 10

typedef struct _struct_Lora_system
{    
    timer_t     Provision_timer;
} struct_Lora_system;
void Lora_Init(void);

void Proc_Lora_Main(void);

uint8_t Provision_pendingTimeout(void);
#endif //PROC_LORA_MAIN_H
