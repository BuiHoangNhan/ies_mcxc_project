#ifndef __MIDDLE_LORA_H
#define __MIDDLE_LORA_H

/**
* @file lora_gw.c
* @brief 
*
* @details 
* @date 2024 Nov 10
* @version 1
* @author Doan Quoc Huy
* Group: Vetek
*
*/
/*=============================================================================*/
#include "Lora_at.h"
#include "lora_msg.h"

#if LCD_CONTROLER

void Lora_SendNodeData(Lora_LCDCtrl_data_t* data);

#elif NODE_NEMA

void Lora_SendNodeData(Lora_Nema_data_t* data);

#endif
void Lora_SendRes_NodeID(void);

void Send_LoraConfirm_Req(Lora_MsgStatus_t status);

void Lora_send_Ack(uint16_t lora_id, Lora_CmdId_t cmd_id);

void Lora_send_Nack(uint16_t lora_id, Lora_CmdId_t cmd_id);
#endif /*__MIDDLE_LORA_H*/