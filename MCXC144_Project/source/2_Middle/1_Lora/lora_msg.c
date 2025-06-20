/**
* @file lora_msg.c
* @brief functions convert lora message to data and  reverse
*
* @details 
* @date 2024 Nov 20
* @version 1
* @author Tran Minh Sang
* Group: Vetek
*
*/

#include "lora_msg.h"
#include "string.h"
#include "crc16.h"
#include "board.h" // just for logging
/*******************************Definitions************************************/

#define MSG2DATA 0x89
#define DATA2MSG 0x98

/*********************************Variables************************************/
static Lora_Msg2Data_cb_t Lora_Msg2Data_cb;
static uint16_t Lora_id;
static uint16_t g_rssi;
static uint16_t count_UPDATE_FW=0;
static uint16_t count_UPDATE_FW2=0;
static uint16_t count_UPDATE_FW1=0;
/*************************Private function prototypes**************************/
static uint8_t lora_checkMsgCrc(uint8_t *msg);
static void lora_parseTypeMsg(uint8_t type, uint8_t *msg, uint8_t *data);
static void lora_parseReqId(uint8_t *msg, Lora_node_infor_t *node);
static void lora_parseResId(uint8_t *msg, Lora_node_config_t  *node);
static void lora_parseDataModel1(uint8_t *msg, Lora_Nema_data_t *light);
static void lora_parseDataModel2(uint8_t *msg, Lora_LCDCtrl_data_t *lcd_ctrl);
static void lora_parseLightCmd(uint8_t *msg, Lora_Nema_control_t *light);
static void lora_parseRelayCmd(uint8_t *msg, Lora_LCDCtrl_control_t *relay);
static void lora_parseFirmware(uint8_t *msg, Lora_Node_firmware_t *fw);
static void lora_parseNemaConfig(uint8_t *msg, Lora_Nema_config_t *nema);
static void lora_parseLCDCtrlConfig(uint8_t *msg, Lora_LCDCtrl_config_t *lcd_ctrl);
static void lora_parseRequestData(uint8_t *msg, Lora_gw_cmd_t *cmd);
static void lora_parseProvision(uint8_t *msg, Lora_gw_cmd_t *cmd);
static void lora_parseOTAMode(uint8_t *msg, Lora_gw_cmd_t *cmd);
static void lora_parseStartOTA(uint8_t *msg, Lora_gw_cmd_t *cmd);
static void lora_parseACK_NACK(uint8_t type, uint8_t *msg, Lora_ACK_NACK_t *status);
/*************************Private function definitions*************************/
static uint8_t lora_checkMsgCrc(uint8_t *msg){
    uint8_t byte_cnt = msg[1];
    uint16_t crc = crc16_calc((void*)(msg), byte_cnt + 3);
    uint16_t msg_crc = msg[byte_cnt + 3] | (msg[byte_cnt + 4] << 8); 
    /* get rssi here */
    g_rssi = msg[byte_cnt + 2];
    // printf("crc: %x %x\n", crc, msg_crc);
    if(msg_crc == crc){
        return 1;
    }
    else{
        return 0;
    }
}

static void lora_parseTypeMsg(uint8_t type, uint8_t *msg, uint8_t *data){
    Lora_CmdId_t cmd_id;
    Lora_MsgType_t byte_cnt;
    if(type == DATA2MSG){
        cmd_id = data[0];
        byte_cnt = data[1];
    }
    else if(type == MSG2DATA){
        cmd_id = msg[0];
        byte_cnt = msg[1];
    }
    switch(cmd_id){
        case REQ_ID:{
            if(byte_cnt == NODE_INFO){
                lora_parseReqId(msg, (Lora_node_infor_t*)(data));
            }
            else if(byte_cnt == NODE_CFG){
                lora_parseResId(msg, (Lora_node_config_t*)(data));
            }
        }
        break;
        case CONFIRM_REQ_ID:{
            lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
        }
        break;
        case SEND_DATA:{
            if(byte_cnt == DATA_NEMA){
                lora_parseDataModel1(msg, (Lora_Nema_data_t*)(data));
            }
            else if(byte_cnt == DATA_LCDCTRL){
                lora_parseDataModel2(msg, (Lora_LCDCtrl_data_t*)(data));
            }            
            else if(byte_cnt == ACK_NACK){
                lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
            }
        }
        break;
        case SEND_CFG:{
            if(byte_cnt == CFG_NEMA){
                lora_parseNemaConfig(msg, (Lora_Nema_config_t*)(data));
            }
            else if(byte_cnt == CFG_LCDCTRL){
                lora_parseLCDCtrlConfig(msg, (Lora_LCDCtrl_config_t*)(data));
            }
            else if(byte_cnt == ACK_NACK){
                lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
            }
        }
        break;
        case SEND_CMD:{
			LOG("Received SEND_CMD\r\n");
            if(byte_cnt == LIGHT_CMD){
				LOG("Received LIGHT_CMD\r\n");
                lora_parseLightCmd(msg, (Lora_Nema_control_t *)(data));
            }
            else if(byte_cnt == RELAY_CMD){
                lora_parseRelayCmd(msg, (Lora_LCDCtrl_control_t*)(data));
            }
            else if(byte_cnt == ACK_NACK){
                lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
            }
        }
        break;
        case REQ_DATA:{
            if (byte_cnt == ACK_NACK){
                lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
            }
            else{
                lora_parseRequestData(msg, (Lora_gw_cmd_t*)(data));
            }
        }
        break;
        case PROVISION:{
            if (byte_cnt == ACK_NACK){
                lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
            }
            else{
                lora_parseProvision(msg, (Lora_gw_cmd_t*)(data));
            }
        }
        break;
        case OTA_MODE:{
            if (byte_cnt == ACK_NACK){
                lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
            }
            else{
                lora_parseOTAMode(msg, (Lora_gw_cmd_t*)(data));
            }
        }
        break;
        case START_OTA:{
            if (byte_cnt == ACK_NACK){
                lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
            }
            else{
                lora_parseStartOTA(msg, (Lora_gw_cmd_t*)(data));
            }
        }
        break;
        case UPDATE_FW:{
            if (byte_cnt == ACK_NACK){
                lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
            }
            else{

            	count_UPDATE_FW++;
                lora_parseFirmware(msg, (Lora_Node_firmware_t*)(data));
            }
        }
        break;
        case 0xAA:{
            lora_parseACK_NACK(type, msg, (Lora_ACK_NACK_t*)(data));
        }
        break;
        default:
        break;
    }
}

static void lora_parseReqId(uint8_t *msg, Lora_node_infor_t *node){
#if(LCD_CONTROLER || NODE_NEMA)
    msg[0] = node->cmd_id;
    msg[1] = node->byte_cnt;
    msg[2] = (node->mac & 0xff);
    msg[3] = (node->mac >> 8) & 0xff;
    msg[4] = (node->mac >> 16) & 0xff;
    msg[5] = (node->mac >> 24) & 0xff;
    msg[6] = (node->model_id) & 0xff;
    msg[7] = (node->model_id >> 8) & 0xff;
    msg[8] = node->fw_ver_major;
    msg[9] = node->fw_ver_minor;
    msg[10] = node->fw_ver_path;
#else
    node->cmd_id = msg[0];
    node->byte_cnt = msg[1];
    node->mac = msg[2] | (msg[3] << 8) | (msg[4] << 26) | (msg[5] << 24);
    node->model_id = msg[6] | msg[7] << 8;
    node->fw_ver_major = msg[8];
    node->fw_ver_minor = msg[9];
    node->fw_ver_path = msg[10];
    // Lora_Msg2Data_cb.lora_requestId_cb(Lora_id, *node);
#endif
}

static void lora_parseResId(uint8_t *msg, Lora_node_config_t *node){
#if(LCD_CONTROLER || NODE_NEMA)
    node->cmd_id = msg[0];
    node->byte_cnt = msg[1];
    node->max_tx_power = msg[2] | (msg[3] << 8); 
    node->reset_rf_time = msg[4];
    node->propagation_radius = msg[5];
    node->CSMA_time = msg[6] | (msg[7] << 8);
    node->router_score = msg[8] | (msg[9] << 8);
    node->data_period = msg[10] | (msg[11] << 8);
    Lora_Msg2Data_cb.lora_respondId_cb(Lora_id, *node);
#else
    msg[0] = node->cmd_id;
    msg[1] = node->byte_cnt;
    msg[2] = node->max_tx_power & 0xff;
    msg[3] = (node->max_tx_power >> 8) & 0xff;
    msg[4] = node->reset_rf_time;
    msg[5] = node->propagation_radius;
    msg[6] = node->CSMA_time & 0xff;
    msg[7] = (node->CSMA_time >> 8) & 0xff;
    msg[8] = node->router_score & 0xff;
    msg[9] = (node->router_score >> 8) & 0xff;
    msg[10] = node->data_period & 0xff;
    msg[11] = (node->data_period >> 8) & 0xff;
#endif
}

static void lora_parseDataModel1(uint8_t *msg, Lora_Nema_data_t *light){
#if NODE_NEMA
    msg[0] = light->cmd_id;
    msg[1] = light->byte_cnt;
    msg[2] = light->U & 0xff;
    msg[3] = (light->U >> 8) & 0xff;
    msg[4] = light->I & 0xff;
    msg[5] = (light->I >> 8) & 0xff;
    msg[6] = light->P & 0xff;
    msg[7] = (light->P >> 8) & 0xff;
    msg[8] = light->temperature & 0xff;
    msg[9] = (light->temperature >> 8) & 0xff;
    msg[10] = light->brightness & 0xff;
    msg[11] = (light->brightness >> 8) & 0xff;
    msg[12] = (light->brightness >> 16) & 0xff;
    msg[13] = (light->brightness >> 24) & 0xff;
    msg[14] = light->uptime & 0xff;
    msg[15] = (light->uptime >> 8) & 0xff;
    msg[16] = (light->uptime >> 16) & 0xff;
    msg[17] = (light->uptime >> 24) & 0xff;
    msg[18] = light->light & 0xff;
    msg[19] = light->alert;
#elif (GATEWAY)
    light->cmd_id = msg[0];
    light->byte_cnt = msg[1];
    light->U = msg[2] | (msg[3] << 8);
    light->I = msg[4] | (msg[5] << 8);
    light->P = msg[6] | (msg[7] << 8);
    light->temperature = msg[8] | (msg[9] << 8);
    light->brightness = msg[10] | (msg[11] << 8) | (msg[12] << 16) | (msg[13] << 24);
    light->uptime = msg[14] | (msg[15] << 8) | (msg[16] << 16) | (msg[17] << 24);
    light->light = msg[18];
    light->alert = msg[19];
    // Lora_Msg2Data_cb.lora_dataFromNema_cb(Lora_id, *light);
#endif
}

static void lora_parseDataModel2(uint8_t *msg, Lora_LCDCtrl_data_t *lcd_ctrl){
#if LCD_CONTROLER 
    msg[0] = lcd_ctrl->cmd_id;
    msg[1] = lcd_ctrl->byte_cnt;
    msg[2] = lcd_ctrl->U1 & 0xff;
    msg[3] = (lcd_ctrl->U1 >> 8) & 0xff;
    msg[4] = lcd_ctrl->I1 & 0xff;
    msg[5] = (lcd_ctrl->I1 >> 8) & 0xff;
    msg[6] = lcd_ctrl->P1 & 0xff;
    msg[7] = (lcd_ctrl->P1 >> 8) & 0xff;
    msg[8] = (lcd_ctrl->P1 >> 16) & 0xff;
    msg[9] = (lcd_ctrl->P1 >> 24) & 0xff;
    msg[10] = lcd_ctrl->U2 & 0xff;
    msg[11] = (lcd_ctrl->U2 >> 8) & 0xff;
    msg[12] = lcd_ctrl->I2 & 0xff;
    msg[13] = (lcd_ctrl->I2 >> 8) & 0xff;
    msg[14] = lcd_ctrl->P2 & 0xff;
    msg[15] = (lcd_ctrl->P2 >> 8) & 0xff;
    msg[16] = (lcd_ctrl->P2 >> 16) & 0xff;
    msg[17] = (lcd_ctrl->P2 >> 24) & 0xff;
    msg[18] = lcd_ctrl->U3 & 0xff;
    msg[19] = (lcd_ctrl->U3 >> 8) & 0xff;
    msg[20] = lcd_ctrl->I3 & 0xff;
    msg[21] = (lcd_ctrl->I3 >> 8) & 0xff;
    msg[22] = lcd_ctrl->P3 & 0xff;
    msg[23] = (lcd_ctrl->P3 >> 8) & 0xff;
    msg[24] = (lcd_ctrl->P3 >> 16) & 0xff;
    msg[25] = (lcd_ctrl->P3 >> 24) & 0xff;
    msg[26] = lcd_ctrl->uptime & 0xff;
    msg[27] = (lcd_ctrl->uptime >> 8) & 0xff;
    msg[28] = (lcd_ctrl->uptime >> 16) & 0xff;
    msg[29] = (lcd_ctrl->uptime >> 24) & 0xff;
    msg[30] = lcd_ctrl->relay1;
    msg[31] = lcd_ctrl->relay2;
    msg[32] = lcd_ctrl->relay3;
    msg[33] = lcd_ctrl->alert;
#elif(GATEWAY)
    lcd_ctrl->cmd_id = msg[0];
    lcd_ctrl->byte_cnt = msg[1];
    lcd_ctrl->U1 = msg[2] | (msg[3] << 8);
    lcd_ctrl->I1 = msg[4] | (msg[5] << 8);
    lcd_ctrl->P1 = msg[6] | (msg[7] << 8) | (msg[8] << 16) | (msg[9] << 24);
    lcd_ctrl->U2 = msg[10] | (msg[11] << 8);
    lcd_ctrl->I2 = msg[12] | (msg[13] << 8);
    lcd_ctrl->P2 = msg[14] | (msg[15] << 8) | (msg[16] << 16) | (msg[17] << 24);
    lcd_ctrl->U3 = msg[18] | (msg[19] << 8);
    lcd_ctrl->I3 = msg[20] | (msg[21] << 8);
    lcd_ctrl->P3 = msg[22] | (msg[23] << 8) | (msg[24] << 16) | (msg[25] << 24);
    lcd_ctrl->uptime = msg[26] | (msg[27] << 8) | (msg[28] << 16) | (msg[29] << 24);
    lcd_ctrl->relay1 = msg[30];
    lcd_ctrl->relay2 = msg[31];
    lcd_ctrl->relay3 = msg[32];
    lcd_ctrl->alert = msg[33];
    // Lora_Msg2Data_cb.lora_dataFromLCDCtrl_cb(Lora_id, *lcd_ctrl);
#endif
}

static void lora_parseLightCmd(uint8_t *msg, Lora_Nema_control_t *light){
#if NODE_NEMA
    light->cmd_id = msg[0];
    light->byte_cnt = msg[1];
    light->value = msg[2];
    Lora_Msg2Data_cb.lora_lightCmd_cb(Lora_id, *light);
#elif(GATEWAY)
    msg[0] = light->cmd_id;
    msg[1] = light->byte_cnt;
    msg[2] = light->value & 0xff;
#endif
}

static void lora_parseRelayCmd(uint8_t *msg, Lora_LCDCtrl_control_t *relay){
#if LCD_CONTROLER
    relay->cmd_id = msg[0];
    relay->byte_cnt = msg[1];
    relay->relay1 = msg[2];
    relay->relay2 = msg[3];
    relay->relay3 = msg[4];
    Lora_Msg2Data_cb.lora_relayCmd_cb(Lora_id, *relay);
#elif(GATEWAY)
    msg[0] = relay->cmd_id;
    msg[1] = relay->byte_cnt;
    msg[2] = relay->relay1;
    msg[3] = relay->relay2;
    msg[3] = relay->relay3;

#endif
}

static void lora_parseFirmware(uint8_t *msg, Lora_Node_firmware_t *fw){
#if(LCD_CONTROLER || NODE_NEMA)
    fw->cmd_id = msg[0];
    fw->byte_cnt = msg[1];
    fw->frame = msg[2] | (msg[3] << 8);
    memcpy((void*)(fw->data), (void*)(&msg[4]), fw->byte_cnt - 2);
    Lora_Msg2Data_cb.lora_firmware_cb(Lora_id, *fw);
#else
    msg[0] = fw->cmd_id;
    msg[1] = fw->byte_cnt;
    msg[2] = fw->frame & 0xff;
    msg[3] = (fw->frame >> 8) & 0xff;
    memcpy((void*)(&msg[4]), (void*)(fw->data), fw->byte_cnt - 2);
#endif
}

static void lora_parseNemaConfig(uint8_t *msg, Lora_Nema_config_t *nema){
#if NODE_NEMA
    nema->cmd_id = msg[0];
    nema->byte_cnt = msg[1];
    nema->vol_thres = msg[2] | (msg[3] << 8);
    nema->current_thres = msg[4] | (msg[5] << 8);
    nema->power_thres = msg[6] | (msg[7] << 8);
    nema->temperature_thres = msg[9] | (msg[9] << 8);
    Lora_Msg2Data_cb.lora_nemaConfig_cb(Lora_id, *nema);
#elif(GATEWAY)
    msg[0] = nema->cmd_id;
    msg[1] = nema->byte_cnt;
    msg[2] = nema->vol_thres & 0xff;
    msg[3] = (nema->vol_thres >> 8) & 0xff;
    msg[4] = nema->current_thres & 0xff;
    msg[5] = (nema->current_thres >> 8) & 0xff;
    msg[6] = nema->power_thres & 0xff;
    msg[7] = (nema->power_thres >> 8) & 0xff;
    msg[8] = nema->temperature_thres & 0xff;
    msg[9] = (nema->temperature_thres >> 8) & 0xff;
#endif
}

static void lora_parseLCDCtrlConfig(uint8_t *msg, Lora_LCDCtrl_config_t *lcd_ctrl){
#if LCD_CONTROLER
    lcd_ctrl->cmd_id = msg[0];
    lcd_ctrl->byte_cnt = msg[1];
    lcd_ctrl->vol_thres = msg[2] | (msg[3] << 8);
    lcd_ctrl->current_thres = msg[4] | (msg[5] << 8);
    lcd_ctrl->power_thres = msg[6] | (msg[7] << 8) | (msg[8] << 16) | (msg[9] << 24);
    lcd_ctrl->latitude = msg[10] | (msg[11] << 8) | (msg[12] << 16) | (msg[13] << 24);
    lcd_ctrl->longitude = msg[14] | (msg[15] << 8) | (msg[16] << 16) | (msg[17] << 24);
    Lora_Msg2Data_cb.Lora_LCDCtrtConfig_cb(Lora_id, *lcd_ctrl);

#elif(GATEWAY)
    msg[0] = lcd_ctrl->cmd_id;
    msg[1] = lcd_ctrl->byte_cnt;
    msg[2] = lcd_ctrl->vol_thres & 0xff;
    msg[3] = (lcd_ctrl->vol_thres >> 8) & 0xff;
    msg[4] = lcd_ctrl->current_thres & 0xff;
    msg[5] = (lcd_ctrl->current_thres >> 8) & 0xff;
    msg[6] = lcd_ctrl->power_thres & 0xff;
    msg[7] = (lcd_ctrl->power_thres >> 8) & 0xff;
    msg[8] = lcd_ctrl->latitude & 0xff;
    msg[9] = (lcd_ctrl->latitude >> 8) & 0xff;
    msg[10] = (lcd_ctrl->latitude >> 16) & 0xff; 
    msg[11] = (lcd_ctrl->latitude >> 24) & 0xff; 
    msg[12] = lcd_ctrl->longitude & 0xff;
    msg[13] = (lcd_ctrl->longitude >> 8) & 0xff;
    msg[14] = (lcd_ctrl->longitude >> 16) & 0xff; 
    msg[15] = (lcd_ctrl->longitude >> 24) & 0xff; 
#endif
}

static void lora_parseRequestData(uint8_t *msg, Lora_gw_cmd_t *cmd){
#if(LCD_CONTROLER || NODE_NEMA)
    Lora_Msg2Data_cb.lora_requestData_cb(Lora_id);
#else
    msg[0] = cmd->cmd_id;
    msg[1] = 0;
#endif
}

static void lora_parseProvision(uint8_t *msg, Lora_gw_cmd_t *cmd){
#if(LCD_CONTROLER || NODE_NEMA)
    Lora_Msg2Data_cb.lora_provision_cb(Lora_id);
#else
    msg[0] = cmd->cmd_id;
    msg[1] = 0;
#endif
}

static void lora_parseOTAMode(uint8_t *msg, Lora_gw_cmd_t *cmd){
#if(LCD_CONTROLER || NODE_NEMA)
    Lora_Msg2Data_cb.lora_otaMode_cb(Lora_id);
#else
    msg[0] = cmd->cmd_id;
    msg[1] = 0;
#endif
}

static void lora_parseStartOTA(uint8_t *msg, Lora_gw_cmd_t *cmd){
#if(LCD_CONTROLER || NODE_NEMA)
    Lora_Msg2Data_cb.lora_startOTA_cb(Lora_id);
#else
    msg[0] = cmd->cmd_id;
    msg[1] = 0;
#endif
}

static void lora_parseACK_NACK(uint8_t type, uint8_t *msg, Lora_ACK_NACK_t *status){
    if(type == MSG2DATA){
        status->cmd_id = msg[0];
        status->byte_cnt = msg[1];
        status->msg_status = msg[2];
        // Lora_Msg2Data_cb.lora_receiveACK_NACK_cb(Lora_id, *status, g_rssi);
    }
    else if(type == DATA2MSG){
        msg[0] = status->cmd_id;
        msg[1] = status->byte_cnt;
        msg[2] = status->msg_status;
    }
}
/*****************************Function definitions*****************************/
void Lora_sendframeOTA(uint8_t status, uint16_t frame, uint8_t *msg, uint32_t *size_msg, int8_t rssi){
    msg[0] = 0x81;
    msg[1] = 0x03;
    msg[2] = frame & 0xff;
    msg[3] = (frame >> 8) & 0xff;
    msg[4] = status;
    msg[5] = rssi;
    uint16_t crc = crc16_calc((void*)(msg), 6);
    msg[6] = crc & 0xff;
    msg[7] = (crc >> 8) & 0xff;
    msg[8] = '\r';
    msg[9] = '\n';
    *size_msg = 10;
}
void Lora_Data2Msg(uint8_t *msg, uint8_t *data, uint32_t *size_msg, int8_t rssi){
    lora_parseTypeMsg(DATA2MSG, msg, data);
    uint8_t byte_cnt = msg[1];
    msg[byte_cnt + 2] = rssi;
    uint16_t crc = crc16_calc((void*)(msg), byte_cnt + 3);
    msg[byte_cnt + 3] = crc & 0xff;
    msg[byte_cnt + 4] = (crc >> 8) & 0xff;
    msg[byte_cnt + 5] = '\r';
    msg[byte_cnt + 6] = '\n';
    *size_msg = byte_cnt + 7;
}

void Lora_Msg2Data(uint16_t lora_id, uint8_t *msg){
    uint8_t data[200];
    Lora_id = lora_id;
    if(lora_checkMsgCrc(msg)){
    	count_UPDATE_FW2++;
#if GATEWAY
        Lora_Msg2Data_cb.lora_recMsg_cb(lora_id, msg, msg[1] + 2, g_rssi);
#else
        lora_parseTypeMsg(MSG2DATA, msg, data);
#endif
    }
    else{
    	count_UPDATE_FW1++;
        Lora_Msg2Data_cb.lora_checkCRCFail_cb(Lora_id);
    }
}

void Lora_RegisterMsg2DataCallback(Lora_Msg2Data_cb_t callback){
    Lora_Msg2Data_cb = callback;
}

#if GATEWAY
void Lora_sendMsgStatus_callback(uint16_t lora_id, uint8_t status, uint8_t msg_tag){
    Lora_Msg2Data_cb.lora_sendMsg_cb(lora_id, status, msg_tag);
}
#endif

/* End of lora_msg.c */
