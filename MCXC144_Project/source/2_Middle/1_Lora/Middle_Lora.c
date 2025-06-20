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
extern int8_t RSSI ;

static uint8_t Lora_Message_buf[200];
#if LCD_CONTROLER
/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Lora_SendNodeData
*Description  : Send data of Node via Lora.
*Input        : Lora_LCDCtrl_data_t* data - The pointer to struct Lora_LCDCtrl_data_t
*Output       : None.
*-----------------------------------------------------------------------------*/
void Lora_SendNodeData(Lora_LCDCtrl_data_t* data)
{
    uint32_t size_message = 0;
    data->cmd_id = SEND_DATA;
    data->byte_cnt = DATA_LCDCTRL;
    Lora_Data2Msg(Lora_Message_buf, (uint8_t*)data, &size_message,Lora_GetRSSI());
    Lora_SendMsg(Lora_Message_buf, size_message);
}
#elif NODE_NEMA
/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Lora_SendNodeData
*Description  : Send data of Node via Lora.
*Input        : Lora_Nema_data_t* data - The pointer to struct Lora_LCDCtrl_data_t
*Output       : None.
*-----------------------------------------------------------------------------*/
void Lora_SendNodeData(Lora_Nema_data_t* data)
{
    uint32_t size_message = 0;
    data->cmd_id = SEND_DATA;
    data->byte_cnt = DATA_NEMA;
    Lora_Data2Msg(Lora_Message_buf, (uint8_t*)data, &size_message,Lora_GetRSSI());
    Lora_SendMsg(Lora_Message_buf, size_message);
    LOG("Node sends data successfully\r\n");
}
#endif
/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Lora_SendRes_NodeID
*Description  : Send request node ID of provision proccess via Lora.
*Input        : None
*Output       : None.
*-----------------------------------------------------------------------------*/
void Lora_SendRes_NodeID(void)
{
	LOG("****** Sending Node ID to broadcast ******\r\n");
    Lora_node_infor_t node;
    node.cmd_id = REQ_ID;
    node.byte_cnt = NODE_INFO;
    Lora_GetMacAddress(&node.mac);
	LOGF("MAC Address: %08X\r\n", node.mac);
    node.model_id = MODEL_ID;
    LOGF("Model ID: %d", MODEL_ID);
    node.fw_ver_major = UserData_getPointer()->SystemData.Fw_ver_major;
    node.fw_ver_minor = UserData_getPointer()->SystemData.Fw_ver_minor;
    node.fw_ver_path = UserData_getPointer()->SystemData.Fw_ver_path;
	LOGF("Firmware version: %d.%d.%d\r\n", UserData_getPointer()->SystemData.Fw_ver_major, UserData_getPointer()->SystemData.Fw_ver_minor, UserData_getPointer()->SystemData.Fw_ver_path);
    uint32_t size_message = 0;
    // This line is to format the data to a message
    Lora_Data2Msg(Lora_Message_buf, &node, &size_message, Lora_GetRSSI());
	// This line is to send the message via Lora
    Lora_SendMsg(Lora_Message_buf, size_message);
    LOG("****** Ended Node ID to broadcast ******\r\n");
}

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Send_LoraConfirm_Req
*Description  : Send Confirm request node via Lora.
*Input        : Lora_MsgStatus_t status - The status of the message.
*Output       : None.
*-----------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Lora_send_Ack
*Description  : Send Ack message when received data via Lora.
*Input        : uint16_t lora_id - id of node.
                Lora_CmdId_t cmd_id  The struct of cmd id 
*Output       : None.
*-----------------------------------------------------------------------------*/
void Lora_send_Ack(uint16_t lora_id, Lora_CmdId_t cmd_id)
{
    Lora_ACK_NACK_t data;
    uint32_t size_message = 0;
    data.cmd_id = cmd_id;
    data.byte_cnt = ACK_NACK;
    data.msg_status = ACK;
    Lora_Data2Msg(Lora_Message_buf, (uint8_t*)&data, &size_message,RSSI);
    Lora_SendMsg(Lora_Message_buf, size_message);
}

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Lora_send_Ack
*Description  : Send Nack message when recieved data via Lora.
*Input        : uint16_t lora_id - id of node.
                Lora_CmdId_t cmd_id  The struct of cmd id 
*Output       : None.
*-----------------------------------------------------------------------------*/
void Lora_send_Nack(uint16_t lora_id, Lora_CmdId_t cmd_id)
{
    Lora_ACK_NACK_t data;
    uint32_t size_message = 0;
    data.cmd_id = 0xAA;
    data.byte_cnt = 1;
    data.msg_status = NACK;
    Lora_Data2Msg(Lora_Message_buf, (uint8_t*)&data, &size_message,RSSI);
    Lora_SendMsg(Lora_Message_buf, size_message);
}

void Lora_send_Nack_OTA(uint8_t status, uint16_t frame)
{    
    Lora_ACK_NACK_t data;
    uint32_t size_message = 0;
    // Lora_Data2Msg(Lora_Message_buf, (uint8_t*)&data, &size_message,RSSI);
    Lora_sendframeOTA( status, frame, Lora_Message_buf, &size_message, RSSI);
    Lora_SendMsg(Lora_Message_buf, size_message);

}
/* End of Lora_at.c */
