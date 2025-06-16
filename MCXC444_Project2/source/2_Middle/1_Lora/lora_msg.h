/**
 * @file lora_msg.h
 * @brief header file
 *
 * @details
 * @date 2024 Nov 20
 * @version 1
 * @author Tran Minh Sang
 * Group: Vetek
 *
 */

 #ifndef LORA_MSG_H_
 #define LORA_MSG_H_
 
 #ifdef __cplusplus
 extern "C"
 {
 #endif
 
 #include <stdint.h>
 /*******************************Definitions************************************/
 
 
 typedef enum{
     REQ_ID = 0xF0,
     CONFIRM_REQ_ID = 0xF1,
     SEND_DATA = 0x03,
     SEND_CFG = 0x04,
     SEND_CMD = 0x05,
     REQ_DATA = 0x07,
     PROVISION = 0x08,
     OTA_MODE = 0x80,
     UPDATE_FW = 0x81,
     START_OTA = 0x82,
 } Lora_CmdId_t;
 
 typedef enum{
     ACK = 0x00,
     NACK = 0xAA
 } Lora_MsgStatus_t;
 
 typedef enum{
     NODE_INFO = 9,
     NODE_CFG = 10,
     DATA_NEMA = 18,
     DATA_LCDCTRL = 32,
     CFG_NEMA = 8,
     CFG_LCDCTRL = 16,
     LIGHT_CMD = 2,
     RELAY_CMD = 3,
     ACK_NACK = 1,
 } Lora_MsgType_t;
 
 typedef enum{
     OVER_VOLTAGE = 1,
     OVER_CURRENT = 2,
     OVER_POWER = 4,
     OVER_HEAT = 8,
 } Lora_AlertType_t;
 
//  typedef uint8_t Lora_CmdId_t;
//  typedef uint8_t Lora_MsgType_t;
//  typedef uint8_t Lora_MsgStatus_t;
//  typedef uint8_t Lora_AlertType_t;
 
 #pragma pack(1)
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         uint32_t mac;
         uint16_t model_id;
         uint8_t fw_ver_major;
         uint8_t fw_ver_minor;
         uint8_t fw_ver_path;
     } Lora_node_infor_t;
 
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         uint16_t max_tx_power;
         uint8_t reset_rf_time;
         uint8_t propagation_radius;
         uint16_t CSMA_time;
         uint16_t router_score;
         uint16_t data_period;
     } Lora_node_config_t;
//     #if NODE_NEMA
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         uint16_t U;
         uint16_t I;
         uint16_t P;
         uint16_t temperature;
         uint32_t brightness;
         uint32_t uptime;
         uint8_t light;
         Lora_AlertType_t alert;	
     } Lora_Nema_data_t;
//    #elif LCD_CONTROLER
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         uint16_t U1;
         uint16_t I1;
         uint32_t P1;
         uint16_t U2;
         uint16_t I2;
         uint32_t P2;
         uint16_t U3;
         uint16_t I3;
         uint32_t P3;
         uint32_t uptime;
         uint8_t relay1;
         uint8_t relay2;
         uint8_t relay3;
         Lora_AlertType_t alert;
     } Lora_LCDCtrl_data_t;
//     #endif
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         uint16_t vol_thres;
         uint16_t current_thres;
         uint16_t power_thres;
         uint16_t temperature_thres;
     } Lora_Nema_config_t;
 
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         uint16_t vol_thres;
         uint16_t current_thres;
         uint32_t power_thres;
         uint32_t latitude;
         uint32_t longitude;
     } Lora_LCDCtrl_config_t;
 
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         uint16_t value;
     } Lora_Nema_control_t;
 
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         uint8_t relay1;
         uint8_t relay2;
         uint8_t relay3;
     } Lora_LCDCtrl_control_t;
 
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         uint16_t frame;
         uint8_t data[140];
     } Lora_Node_firmware_t;
 
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
     } Lora_gw_cmd_t;
 
     typedef struct
     {
         Lora_CmdId_t cmd_id;
         Lora_MsgType_t byte_cnt;
         Lora_MsgStatus_t msg_status;
     } Lora_ACK_NACK_t;
 #pragma pack()
 
 typedef struct{
 #if(LCD_CONTROLER || NODE_NEMA)
     void (*lora_respondId_cb)(uint16_t lora_id, Lora_node_config_t data);
     void (*lora_firmware_cb)(uint16_t lora_id, Lora_Node_firmware_t data);
     void (*lora_requestData_cb)(uint16_t lora_id);
     void (*lora_provision_cb)(uint16_t lora_id);
     void (*lora_otaMode_cb)(uint16_t lora_id);
     void (*lora_startOTA_cb)(uint16_t lora_id);
 #endif
 
 #if  NODE_NEMA
     void (*lora_lightCmd_cb)(uint16_t lora_id, Lora_Nema_control_t data);
     void (*lora_nemaConfig_cb)(uint16_t lora_id, Lora_Nema_config_t data);
 #elif LCD_CONTROLER
     void (*lora_relayCmd_cb)(uint16_t lora_id, Lora_LCDCtrl_control_t data);
     void (*Lora_LCDCtrtConfig_cb)(uint16_t lora_id, Lora_LCDCtrl_config_t data);
 #elif defined(GATEWAY)
     // void (*lora_requestId_cb)(uint16_t lora_id, Lora_node_infor_t data);
     // void (*lora_dataFromNema_cb)(uint16_t lora_id, Lora_Nema_data_t data);
     // void (*lora_dataFromLCDCtrl_cb)(uint16_t lora_id, Lora_LCDCtrl_data_t data);
 
     /**
      * @brief callback when receive data from source lora
      * @param lora_id is source id 
      * @param data is data from source lora
      * @param rssi
      * 
      */
     void (*lora_recMsg_cb)(uint16_t lora_id, uint8_t *data, uint8_t len, int8_t rssi);
 
     /**
      * @brief callback when send data to destination lora
      * @param lora_id is dest id
      * @param status: 9 -> SUCCESS, 5 -> NO ROUTE, 6 -> NO ACK
      * @param msg_tag 
      * 
      */
     void (*lora_sendMsg_cb)(uint16_t lora_id, uint8_t status, uint8_t msg_tag);
     #endif
 
     void (*lora_receiveACK_NACK_cb)(uint16_t lora_id, Lora_ACK_NACK_t status, int8_t rssi);
 
     /**
      * @brief callback when receive msg with fail CRC
      * @param lora_id is source id
      * 
      */
     void (*lora_checkCRCFail_cb)(uint16_t lora_id);
     
 }Lora_Msg2Data_cb_t;
 
 /****************************Function prototypes*******************************/
     void extern Lora_Data2Msg(uint8_t *msg, uint8_t *data, uint32_t *size_msg, int8_t rssi);
     void extern Lora_Msg2Data(uint16_t lora_id, uint8_t *msg);
     void extern Lora_RegisterMsg2DataCallback(Lora_Msg2Data_cb_t callback);
 
 #ifdef GATEWAY
     /* this function used for lora_gw_uart, user don't use and don't care*/
     void extern Lora_sendMsgStatus_callback(uint16_t lora_id, uint8_t status, uint8_t msg_tag);
 #endif
 
     /******************************************************************************/
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* LORA_MSG_H_ */
 
 /* End of lora_msg.h */
 
