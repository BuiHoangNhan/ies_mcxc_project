/**
* @file Middle_Lora.c
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
#include "Middle_Lora.h"

static uint8_t Lora_Message_buf[200];
#if LCD_CONTROLER
void Lora_SendNodeData(Lora_LCDCtrl_data_t* data)
{
    uint32_t size_message = 0;
    data->cmd_id = SEND_DATA;
    data->byte_cnt = DATA_LCDCTRL;
    Lora_Data2Msg(Lora_Message_buf, (uint8_t*)data, &size_message,Lora_GetRSSI());
    Lora_SendMsg(Lora_Message_buf, size_message);
}
#elif NODE_NEMA
void Lora_SendNodeData(Lora_Nema_data_t* data)
{
    uint32_t size_message = 0;
    data->cmd_id = SEND_DATA;
    data->byte_cnt = DATA_NEMA;
    Lora_Data2Msg(Lora_Message_buf, (uint8_t*)data, &size_message,Lora_GetRSSI());
    Lora_SendMsg(Lora_Message_buf, size_message);
}
#endif
void Lora_SendRes_NodeID(void)
{
    
    Lora_node_infor_t node;
    node.cmd_id = REQ_ID;
    node.byte_cnt = NODE_INFO;
    Lora_GetMacAddress(&node.mac);
#if LCD_CONTROLER
    node.model_id = 2;
#elif NODE_NEMA
    node.model_id = 1;
#endif
    node.fw_ver_major = 0;
    node.fw_ver_minor = 0;
    node.fw_ver_path = 1;
    //TODO add missing aguments
    uint32_t size_message = 0;
    Lora_Data2Msg(Lora_Message_buf, &node, &size_message,Lora_GetRSSI());
    Lora_SendMsg(Lora_Message_buf, size_message);
}

void Send_LoraConfirm_Req(Lora_MsgStatus_t status)
{
    Lora_ACK_NACK_t confirm_data;
    confirm_data.cmd_id = CONFIRM_REQ_ID;
    confirm_data.byte_cnt = ACK_NACK;
    confirm_data.msg_status = status;
    uint32_t size_message = 0;
    Lora_Data2Msg(Lora_Message_buf, &confirm_data, &size_message,Lora_GetRSSI());
    Lora_SendMsg(Lora_Message_buf, size_message);
}
void Lora_send_Ack(uint16_t lora_id, Lora_CmdId_t cmd_id)
{
    Lora_ACK_NACK_t data;
    uint32_t size_message = 0;
    data.cmd_id = cmd_id;
    data.byte_cnt = ACK_NACK;
    data.msg_status = ACK;
    Lora_Data2Msg(Lora_Message_buf, (uint8_t*)&data, &size_message,0);
    Lora_SendMsg(Lora_Message_buf, size_message);
}
void Lora_send_Nack(uint16_t lora_id, Lora_CmdId_t cmd_id)
{
    Lora_ACK_NACK_t data;
    uint32_t size_message = 0;
    data.cmd_id = cmd_id;
    data.byte_cnt = 1;
    data.msg_status = NACK;
    Lora_Data2Msg(Lora_Message_buf, (uint8_t*)&data, &size_message,Lora_GetRSSI());
    Lora_SendMsg(Lora_Message_buf, size_message);
}
/* End of Lora_at.c */
