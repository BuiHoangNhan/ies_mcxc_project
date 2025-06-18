#include "Proc_Lora_main.h"
#include <3_Application/Sys_Process.h>
#include <2_Middle/4_Sensor/Sensor.h>
#include <2_Middle/8_user_data/user_data.h>
#include <2_Middle/7_ota/ota.h>
#include <2_Middle/7_ota/platform_bindings.h>
#include <2_Middle/7_ota/mcuboot_app_support.h>
static const uint16_t ChannelID_list[MAX_NBR_PAINID] = {3, 11, 19, 27, 35, 43, 51, 59, 67, 75};
static const uint16_t PainID_list[MAX_NBR_PAINID] = {101, 5087, 10987, 17293, 23753, 30529, 37309, 44249, 51413, 58481};
static uint16_t count = 0;
enum_Provision_state *Lora_Provision_Succes(void);
static struct_Lora_system StrLora_system;
static uint8_t index = 0;
static uint16_t Ota_Frame = 0;
static uint64_t vPr_pendingTimeout = 0;

#define PROVISION_TIMEOUT 3000
void Lora_set_toProvision(void);
static void Lora_Revc_resqData(uint16_t lora_id)
{
    Proc_Change_ProcCmd(PROC_LORA, CMD_LORA_SEND_DATA);
}
static void Lora_FW_cb_Revc(uint16_t lora_id, Lora_Node_firmware_t fw)
{   if(OTA_getState() == eOTA_Progress)
    {
        uint8_t *p_fwBuff = fw.data;
        if(fw.frame == Ota_Frame)
        {
            OTA_refreshTimer();
            Ota_Frame++;
            OTA_progress(p_fwBuff,fw.byte_cnt-2);
            // Lora_send_Ack(0, fw.cmd_id);
            Lora_send_Nack_OTA(0, fw.frame);
        }   
        else
        {
            Lora_send_Nack_OTA(0xAA, fw.frame);
        }
    }
}
#if LCD_CONTROLER

static void LCD_Controler_Config(uint16_t lora_id, Lora_LCDCtrl_config_t data)
{
    UserData_getPointer()->SystemData.Provision_state = PROVISION_SUCCESS;
    UserData_getPointer()->Node_userConfig.current_thres = data.current_thres/100;
    UserData_getPointer()->Node_userConfig.vol_thres = data.vol_thres/100;
    UserData_getPointer()->Node_userConfig.power_thres = data.power_thres/100;
    UserData_getPointer()->Node_userConfig.latitude = data.latitude;
    UserData_getPointer()->Node_userConfig.longitude = data.longitude;
    UserData_save();
    uint8_t LCDtemp_buff[18] = {0, UserData_getPointer()->SystemData.Fw_ver_major, 0, UserData_getPointer()->SystemData.Fw_ver_minor, 0, UserData_getPointer()->SystemData.Fw_ver_path,\
                                (UserData_getPointer()->SystemData.Mac&0xFF000000)>>24,\
                                (UserData_getPointer()->SystemData.Mac&0x00FF0000)>>16,\
                                (UserData_getPointer()->SystemData.Mac&0x0000FF00)>>8,\
                                (UserData_getPointer()->SystemData.Mac&0xFF),\
                                ((int)data.latitude&0xFF000000)>>24,((int)data.latitude&0x00FF0000)>>16,\
                                ((int)data.latitude&0x0000FF00)>>8,((int)data.latitude&0x000000FF),\
                                ((int)data.longitude&0xFF000000)>>24,((int)data.longitude&0x00FF0000)>>16,\
                                ((int)data.longitude&0x0000FF00)>>8,((int)data.longitude&0x000000FF)}; 
	Proc_DWIN(GOTO_PAGE, 0,0, 0,0x0);
    Proc_DWIN(WRITE_LCD_INIT_INFO, LCD_VERSIOM_MAJOR,(uint8_t*)LCDtemp_buff, 0,0x01);
	Lora_send_Ack(0, data.cmd_id);
}

#elif NODE_NEMA

static void NODE_NEMA_Config(uint16_t lora_id ,Lora_Nema_config_t data)
{
    UserData_getPointer()->SystemData.Provision_state = PROVISION_SUCCESS;
    UserData_getPointer()->Node_userConfig.current_thres = data.current_thres/100;
    UserData_getPointer()->Node_userConfig.vol_thres = data.vol_thres/100;
    UserData_getPointer()->Node_userConfig.power_thres = data.power_thres/100;
    UserData_save();
	Lora_send_Ack(0, data.cmd_id);
}
#endif
void Lora_set_toProvision(void)
{
	if(UserData_getPointer()->SystemData.Provision_state == PROVISION_SUCCESS)
	{
		UserData_getPointer()->SystemData.Provision_state = PROVISION_NONE;
		UserData_getPointer()->SystemData.PainID = 0;
		UserData_getPointer()->SystemData.ChannelID = 0;
		UserData_getPointer()->SystemData.Dst_addr =0;
		UserData_save();
	}
}
static void Lora_Provision_Revc(uint16_t lora_id)
{
	Lora_set_toProvision();
	Lora_send_Ack(0, PROVISION);
	System_DelayMs(1*1000);
}
static void Lora_OTA_Revc(uint16_t lora_id)
{
    OTA_refreshTimer();
    Ota_Frame = 0;
    OTA_start();
    Lora_send_Ack(0, 0X80);
    OTA_setState(eOTA_Progress);
//    Proc_Change_ProcCmd(PROC_LORA, CMD_LORA_PARSE_FW);
}
static void Lora_StartOTA_Revc(uint16_t lora_id)
{
    if(OTA_getState() == eOTA_Progress)
    {
        OTA_setState(eOTA_Idle);
        Lora_send_Ack(0, START_OTA);
        OTA_done();
        OTA_setState(eOTA_Idle);
    }
}
#if LCD_CONTROLER
static void Lora_Revc_RelayCmd(uint16_t lora_id, Lora_LCDCtrl_control_t relay)
{
    Sensor_t *Sensor = Sensor_get_value();
    Sensor->Relay[0] = relay.relay1;
    Sensor->Relay[1] = relay.relay2;
    Sensor->Relay[2] = relay.relay3;
    Proc_Change_ProcCmd(PROC_RELAY, CMD_RELAY);
    Sensor->Relay_isChange = RELAY_CHANGE_BY_MSG;
    Lora_send_Ack(0, relay.cmd_id); 
}
#elif NODE_NEMA
void Lora_Revc_LightCmd(uint16_t lora_id, Lora_Nema_control_t light)
{
    Sensor_t *Sensor = Sensor_get_value();
    Sensor->Dim_value = light.value;
    Lora_send_Ack(0, light.cmd_id); 
    Proc_Change_ProcCmd(PROC_DIMMER, CMD_DIMMER);
}
#endif
void Lora_receiveACK_NACK(uint16_t lora_id, Lora_ACK_NACK_t status)
{
    switch (status.cmd_id)
    {
    default:
        break;
    }
}
void Lora_Provision(void)
{
    enum Lora_message_t *result = Lora_GetMsgResult();
    if(index == 0)
    {
       Lora_ChangMode(Broadcast);
       *result=MSG_NONE;
#if LCD_CONTROLER
        Proc_DWIN(GOTO_PAGE, 0,0, 0,0x01);
#endif
    }
    if(*result==MSG_SUCCESS)
    {
    	Lora_SetMsgResult(MSG_NONE);
    	Timer_Create(&StrLora_system.Provision_timer, PROVISION_TIMEOUT);
    }
    else if(Timer_Timeout(&StrLora_system.Provision_timer))
    {
    	if(index == MAX_NBR_PAINID)
		{
			index = 0;
		}
        Lora_SetChannel(ChannelID_list[index]);
        Lora_SetPainID(PainID_list[index]);
        System_DelayMs(500);
        // This function is for the Node send their NodeID
        Lora_SendRes_NodeID();
        index++;
    }   
}
static void Lora_respondId_Revc(uint16_t lora_id, Lora_node_config_t data)
{
    enum_Provision_state *Provision_status = Lora_Provision_Succes(); 
    UserData_getPointer()->NodeConfig.CSMA_time = data.CSMA_time;
    UserData_getPointer()->NodeConfig.reset_rf_time = data.reset_rf_time;
    UserData_getPointer()->NodeConfig.max_tx_power = data.max_tx_power;
    UserData_getPointer()->NodeConfig.router_score = data.router_score;
    UserData_getPointer()->NodeConfig.data_period = data.data_period;
    UserData_getPointer()->NodeConfig.propagation_radius = data.propagation_radius;
    UserData_getPointer()->SystemData.PainID = Lora_data_revc.PainID;
    for(uint8_t i=0 ; i<MAX_NBR_PAINID;i++)
    {
        if(Lora_data_revc.PainID == PainID_list[i])
        {
            UserData_getPointer()->SystemData.ChannelID = ChannelID_list[i];   
            break;
        }
    }
    index = 0;
    Lora_SetChannel(UserData_getPointer()->SystemData.ChannelID);
    Lora_SetPainID(UserData_getPointer()->SystemData.PainID);
    UserData_save();
    if(*Provision_status == PROVISION_NONE)
    {
        *Provision_status = PROVISION_PENDING;
    }
    Timer_Create(&vPr_pendingTimeout, 1000*60*2);
    Proc_Change_ProcCmd(PROC_LORA, CMD_LORA_CF_REQ_ID);
    Lora_configCondition();
    
}

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Lora_Init
*Description  : Initializes the LoRa module.
*Input        : None
*Output       : None
*-----------------------------------------------------------------------------*/

void Lora_Init(void)
{
	LOG("Lora_Init Started\r\n");
    Lora_Msg2Data_cb_t callback ={
        .lora_requestData_cb = Lora_Revc_resqData,
        .lora_firmware_cb = Lora_FW_cb_Revc,
        .lora_provision_cb = Lora_Provision_Revc,
        .lora_otaMode_cb = Lora_OTA_Revc,
        .lora_startOTA_cb = Lora_StartOTA_Revc,
        .lora_respondId_cb = Lora_respondId_Revc,
        .lora_receiveACK_NACK_cb = Lora_receiveACK_NACK,
        .lora_checkCRCFail_cb = Lora_send_Nack,
#if LCD_CONTROLER
        .lora_relayCmd_cb = Lora_Revc_RelayCmd,
        .Lora_LCDCtrtConfig_cb = LCD_Controler_Config,
#elif NODE_NEMA
        .lora_nemaConfig_cb = NODE_NEMA_Config,
        .lora_lightCmd_cb = Lora_Revc_LightCmd,
#endif
    } ;
    Lora_RegisterMsg2DataCallback(callback);
    Lora_uart_init();
    Lora_rst_init();
    // This function is to configure the LoRa, and establish basic parameters for communication.
    Lora_at_init();
	LOG("Lora_Init Finished\r\n");
}

void Proc_Lora_Main(void)
{
    switch (Proc_GetCmd())
    {
    case CMD_LORA_SEND_REQ_NODEID:
		LOG("Node starts sending NODEID\r\n");
    	Lora_SendRes_NodeID();
        break;
    case CMD_GW_SEND_CONFIG:
        break;
    case CMD_LORA_SEND_DATA:
#if   LCD_CONTROLER
        Lora_LCDCtrl_data_t Node_data;
#elif NODE_NEMA
        Lora_Nema_data_t Node_data;
#endif
        Sensor_update_NodeValue(&Node_data); // Send data from sensor
        Lora_SendNodeData(&Node_data); // This function is to send that data
        break;
    case CMD_LORA_SEND_ACK:
        break;
    case CMD_LORA_SEND_NACK:
        break;
    case CMD_LORA_PROVISION:
            // This case is when Node send  their broadcast ID, and require for (PAN ID, channel)       
            Lora_Provision();
        break;
    case CMD_LORA_CF_REQ_ID:
    	Lora_ChangMode(Unicast);
    	System_DelayMs(1000);
    	Lora_send_Ack(0,CONFIRM_REQ_ID);
        break;
    case CMD_LORA_PARSE_FW:
        break;
    case CMD_LORA_START_OTA:
        break;
    default:
        break;
    }
}
enum_Provision_state *Lora_Provision_Succes(void)
{
    UserData* pUserData = UserData_getPointer();
    return &pUserData->SystemData.Provision_state;
}
uint8_t Provision_pendingTimeout(void)
{
	return Timer_Timeout(&vPr_pendingTimeout);
}
