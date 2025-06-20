#include "Lora_at.h"
#include "lora_msg.h"
#include <1_Hardware/0_Timer/timer.h>
#include <2_Middle/8_user_data/user_data.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*=============================================================================
*
*   @section Macros
*
/=============================================================================*/
#define Lora_AT_TIMEOUT_MS (10000)

/*=============================================================================
*
*   @section Private data
*
/=============================================================================*/

static uint32_t Lora_mac_addr = 0;
static uint16_t Lora_painID = {0x00};
static uint16_t Lora_src_addr = {0x00};
static uint16_t *Lora_dst_addr;
extern UserData g_userData;

static uint8_t statusBreak = 0;
static uint32_t bt_raw_len = 0;
static uint8_t bt_has_data = 0;
uint8_t buf[200];
Data_revc_t Lora_data_revc;
enum Lora_message_t Lora_msg_result;
static str_NodeRoute vRouting;

static const struct rx_status_tok rx_st_l[] = {

    TOK_AND_MSG("CMD_VALUE_ERR", MSG_ERROR),
    TOK_AND_MSG("NO ROUTE", MSG_NO_ROUTE),
    TOK_AND_MSG("ACK", MSG_NO_ACK),
    TOK_AND_MSG("OK", MSG_OK),
    TOK_AND_MSG("SUCCESS", MSG_SUCCESS),
    TOK_AND_MSG("NO.  DST    HOP    SC  RSSI", MSG_RSSI),
    //        TOK_AND_MSG("\r\n", MSG_LINE_BREAK),
    // TOK_AND_MSG((uint8_t *)0XAA, MSG_SEND_OK),
    // TOK_AND_MSG(0, MSG_SEND_FAIL),

};
#define RX_TOK_LIST_SIZE (sizeof(rx_st_l) / sizeof(*(rx_st_l)))

static const uint8_t *txt_at_cmd_at = "AT";

/*AT COMMAND QUERY */
static const uint8_t *txt_at_reset = "AT+RESET";
static const uint8_t *txt_at_default = "AT+DEFAULT";
static const uint8_t *txt_at_info = "AT+INFO=?";
static const uint8_t *txt_at_get_power = "AT+POWER=?";
static const uint8_t *txt_at_get_uart = "AT+UART=?";
static const uint8_t *txt_at_get_rate = "AT+RATE=?";
static const uint8_t *txt_at_option_get = "AT+OPTION=?";
static const uint8_t *txt_at_get_painID = "AT+PANID=?";
static const uint8_t *txt_at_get_src_adr = "AT+SRC_ADDR=?";
static const uint8_t *txt_at_get_dst_adr = "AT+DST_ADDR=?";
static const uint8_t *txt_at_gr_adr_get = "AT+GROUP_ADD=?";
static const uint8_t *txt_at_query_mac = "AT+MAC=?";
static const uint8_t *txt_at_get_rssi = "AT+ROUTER_READ=?";
/*AT COMMAND SET */
static const uint8_t *txt_at_power_set = "AT+POWER=%d,%d";
static const uint8_t *txt_at_uart_set = "AT+UART=%d,%d";
static const uint8_t *txt_at_rate_set = "AT+RATE=%d";
static const uint8_t *txt_at_option_set = "AT+OPTION=%d,%d";
static const uint8_t *txt_at_painID_set = "AT+PANID=%d,%d";
static const uint8_t *txt_at_set_src_adr = "AT+SRC_ADDR=%d,%d";
static const uint8_t *txt_at_set_dst_adr = "AT+DST_ADDR=%d,%d";
static const uint8_t *txt_at_gr_adr_set = "AT+GROUP_ADD=%d";
static const uint8_t *txt_at_gr_adr_del = "AT+GROUP_DEL=%d";
static const uint8_t *txt_at_channel_set = "AT+CHANNEL=%d,%d";
static const uint8_t *txt_at_reset_time = "AT+RESET_TIME=%d";
static const uint8_t *txt_at_csma_rng = "AT+CSMA_RNG=%d";
static const uint8_t *txt_at_router_score = "AT+ROUTER_SCORE=%d";
static const uint8_t *txt_at_member_rad = "AT+MEMBER_RAD=%d,%d";
static const uint8_t *txt_at_nonmember_rad = "AT+NONMEMBER_RAD=%d,%d";

static uint32_t Lora_at_send(uint8_t *cmdData, uint32_t dataSize);
static uint8_t *getCmdTextByCmd(eAtCmd cmd, uint8_t *message, uint32_t input, enum_save_t save_type, uint32_t input_size);
static enum Lora_message_t rx_tok_matching(uint8_t *buf, uint32_t bs);

#define RX_PROCESS_BUF_SIZE 512
static uint8_t Lora_buf[RX_PROCESS_BUF_SIZE];
static uint8_t cmdBuf[128];

int8_t RSSI = -55;

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: rx_tok_matching
 *Description  : Matches the received token in the buffer.
 *Input        : uint8_t *buf - Pointer to the buffer containing the received token.
 *               uint32_t bs - Size of the buffer.
 *Output       : enum Lora_message_t - The matched Lora message type.
 *-----------------------------------------------------------------------------*/

static enum Lora_message_t rx_tok_matching(uint8_t *buf, uint32_t bs)
{
        if (bs == 0)
                return MSG_NONE;

        enum Lora_message_t msg = MSG_NONE;
        //        bool stop = false;
        //        uint32_t bi = 0;
        //        uint32_t li = 0;
        //        while (!stop && bi < bs && li < RX_TOK_LIST_SIZE) {
        //                if (buf[bi] != rx_st_l[li].tok[bi]) {
        //                       li++;
        //                }
        //                else {
        //                        bi++;
        //                        if (bi == rx_st_l[li].tok_len) {
        //                                msg = rx_st_l[li].msg;
        //                                stop = true;
        //                        }
        //                }
        //                if (li >= RX_TOK_LIST_SIZE) {
        //                        stop = true;
        //                }
        //        }
        for (uint8_t i = 0; i < RX_TOK_LIST_SIZE; i++)
        {
                if (strstr((char *)buf, rx_st_l[i].tok) != NULL)
                {
                        msg = rx_st_l[i].msg;
                        break;
                }
        }
        return msg;
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: getCmdTextByCmd
 *Description  : Retrieves the command text based on the provided command and parameters.
 *Input        : eAtCmd cmd - The command type.
 *               uint32_t input - The input value for the command.
 *               enum_save_t save_type - The save type for the command.
 *               uint32_t input_size - The size of the input.
 *Output       : uint8_t* - Pointer to the command text.
 *-----------------------------------------------------------------------------*/

static uint8_t *getCmdTextByCmd(eAtCmd cmd, uint8_t *message, uint32_t input, enum_save_t save_type, uint32_t input_size)
{
        uint8_t *buf = NULL;
        uint8_t *text = NULL;
        switch (cmd)
        {

        case AT_CMD_AT:
                text = (uint8_t *)txt_at_cmd_at;
                break;
        case AT_CMD_RESET:
                text = (uint8_t *)txt_at_reset;
                break;
        case AT_CMD_DEFAULT:
                text = (uint8_t *)txt_at_default;
                break;
        case AT_CMD_POWER_GET:
                text = (uint8_t *)txt_at_get_power;
                break;
        case AT_CMD_RATE_GET:
                text = (uint8_t *)txt_at_get_rate;
                break;
        case AT_CMD_OPTION_GET:
                text = (uint8_t *)txt_at_option_get;
                break;
        case AT_CMD_PAINID_GET:
                text = (uint8_t *)txt_at_get_painID;
                break;
        case AT_CMD_GET_SRCADDR:
                text = (uint8_t *)txt_at_get_src_adr;
                break;

        case AT_CMD_GET_DSTADDR:
                text = (uint8_t *)txt_at_get_dst_adr;
                break;

        case AT_CMD_GET_GR_ADDR:
                text = (uint8_t *)txt_at_gr_adr_get;
                break;
        case AT_CMD_QUERY_MAC:
                text = (uint8_t *)txt_at_query_mac;
                break;

        case AT_CMD_GET_RSSI:
                text = (uint8_t *)txt_at_get_rssi;
                break;

        case AT_CMD_POWER_SET:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_power_set, input, save_type);
                        text = buf;
                }
                break;

        case AT_CMD_UART_SET:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_uart_set, input, save_type);
                        text = buf;
                }
                break;

        case AT_CMD_RATE_SET:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_rate_set, input);
                        text = buf;
                }
                break;

        case AT_CMD_OPTION_SET:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_option_set, (eLora_option)input, save_type);
                        text = buf;
                }
                break;

        case AT_CMD_PAINID_SET:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_painID_set, (uint16_t)input, save_type);
                        text = buf;
                }
                break;
        case AT_CMD_SET_CHANNEL:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_channel_set, (uint16_t)input, save_type);
                        text = buf;
                }
                break;
        case AT_CMD_SET_SRCADDR:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_set_src_adr, (uint16_t)input, save_type);
                        text = buf;
                }
                break;

        case AT_CMD_SET_DSTADDR:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_set_dst_adr, (uint16_t)input, save_type);
                        text = buf;
                }
                break;
        case AT_CMD_SET_GR_ADDR:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_gr_adr_set, input);
                        text = buf;
                }
                break;

        case AT_CMD_GR_DEL:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_gr_adr_del, input);
                        text = buf;
                }
                break;
        case AT_CMD_SEND_MESSAGE:
                if (message)
                        text = message;
                break;
        case AT_CMD_RESET_TIME:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_reset_time, input);
                        text = buf;
                }
                break;
        case AT_CMD_CSMA_RNG:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_csma_rng, input);
                        text = buf;
                }
                break;
        case AT_CMD_ROUTER_SCORE:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_router_score, input);
                        text = buf;
                }
                break;
        case AT_CMD_MEMBER_RAD:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_member_rad, (uint16_t)input, save_type);
                        text = buf;
                }
                break;
        case AT_CMD_NONMEMBER_RAD:
                buf = cmdBuf;
                if (buf)
                {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_nonmember_rad, (uint16_t)input, save_type);
                        text = buf;
                }
                break;
        default:
                break;
        }

        return text;
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_at_cmdSync
 *Description  : Executes a synchronous LoRa AT command with the specified parameters.
 *Input        : eAtCmd cmd - The command type.
 *               uint32_t input - The input value for the command.
 *               enum_save_t save_type - The save type for the command.
 *               uint32_t input_size - The size of the input.
 *               atcbfn cbfn - Callback function for the command.
 *               void* atcbpr - Pointer to the callback parameter.
 *               uint32_t timeout - Timeout value for the command.
 *Output       : uint32_t - Result of the command execution.
 *-----------------------------------------------------------------------------*/

uint32_t Lora_at_cmdSync(eAtCmd cmd, uint8_t *message, uint32_t input, enum_save_t save_type, uint32_t input_size, atcbfn cbfn, void *atcbpr, uint64_t timeout)
{
        uint32_t status = 0;
        uint64_t cmdTime = Lora_AT_TIMEOUT_MS;
        uint8_t *text = getCmdTextByCmd(cmd, message, input, save_type, input_size);
        uint8_t done = 0;
        Lora_uart_main();
        if (timeout)
        {
                cmdTime = timeout;
        }

        /* This made makes RX fails to receive LD stop streaming command
        uart_drv_buffer_clear();
        */
        if (text)
        {
                timer_t cmdTimer;
                Timer_Create(&cmdTimer, cmdTime);
                if (cmd == AT_CMD_SEND_MESSAGE)
                {
                        Lora_at_send(text, input_size);
                }
                else
                {
                        Lora_at_send(text, strlen(text));
                }
                statusBreak = 0;
                while (!Timer_Timeout(&cmdTimer) && !statusBreak && !done)
                {
                        Lora_at_rxProcess(cbfn, atcbpr, &done, cmd);
                }
        }

        return (uint32_t)done;
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_at_send
 *Description  : Sends the specified command data to the LoRa module.
 *Input        : uint8_t* cmdData - Pointer to the command data to be sent.
 *               uint32_t dataSize - Size of the command data.
 *Output       : uint32_t - Result of the send operation.
 *-----------------------------------------------------------------------------*/

static uint32_t Lora_at_send(uint8_t *cmdData, uint32_t dataSize)
{
        uart_put_bytes(cmdData, dataSize);
        return 0;
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_at_rxProcess
 *Description  : Processes the received data from the LoRa module.
 *Input        : atcbfn fn - Callback function for processing the received data.
 *               void* atcbpr - Pointer to the callback parameter.
 *               uint8_t* done - Pointer to the flag indicating completion.
 *               eAtCmd cmd - The command type.
 *Output       : None
 *-----------------------------------------------------------------------------*/

void Lora_at_rxProcess(atcbfn fn, void *atcbpr, uint8_t *done, eAtCmd cmd)
{
        static uint8_t overflow = 0;
        uint8_t *buf = Lora_buf;
        memset(buf, 0x00, RX_PROCESS_BUF_SIZE);
        uint8_t lend[2] = {0x00};
        uint8_t c = 0;
        uint32_t cnt = 0;
        uint32_t ret;
        Data_revc_t MsgData;
        memset(&MsgData, 0x00, sizeof(Data_revc_t));
        do
        {
                c = 0;
                ret = uart_get_char(&c);
                if (ret)
                {
                        buf[cnt++] = c;
                        if (cmd == AT_CMD_NONE && cnt == 8)
                        {
                                parseMessageBuffer(buf, &MsgData);
                        }
                        if (c == '\r')
                        {
                                if (cmd == AT_CMD_NONE)
                                {
                                        if (cnt != 0 && MsgData.content_length && cnt == (MsgData.content_length + 8 - 1))
                                        {
                                                lend[0] = c;
                                        }
                                }
                                else
                                {
                                        lend[0] = c;
                                }
                        }
                        else if (c == '\n' && lend[0] == '\r')
                        {
                                lend[1] = '\n'; // Completed line ending
                        }
                        else
                        { // Reset
                                lend[0] = 0;
                                lend[1] = 0;
                        }
                }

                if (lend[1] == '\n')
                {
                        ret = 0;
                }
                else if (!ret && cnt)
                {
                        // Best effort to read more data
                        timer_t rxto;
                        Timer_Create(&rxto, 50);
                        uint8_t t;
                        while (!Timer_Timeout(&rxto))
                        {
                                // Continue to get if there is still data
                                if (uart_peek_char(&t))
                                {
                                        ret = 1;
                                }
                        }
                }
        } while (ret != 0 && cnt < RX_PROCESS_BUF_SIZE);

        if (cnt)
        {
                LOGF("Lora Received Data (%lu bytes): ", cnt);
                for (uint32_t i = 0; i < cnt; i++)
                {
                        LOGF("%02X ", buf[i]); // Print each byte in hex format
                }
                LOG("\r\n");
        }
        switch (rx_tok_matching(buf, cnt))
        {
        case MSG_ERROR:
                if (done)
                {
                        *done = 1;
                }
                break;

        case MSG_OK:
                if (done && cmd != AT_CMD_GET_RSSI)
                {
                        *done = 1;
                }
                break;
        case MSG_NO_ROUTE:
                if (done)
                {
                        *done = 1;
                }
                Lora_msg_result = MSG_NO_ROUTE;
                break;
        case MSG_SUCCESS:
                if (done)
                {
                        *done = 1;
                }
                Lora_msg_result = MSG_SUCCESS;

                break;
        case MSG_RSSI:

                break;
        case MSG_NO_ACK:
                if (done)
                {
                        *done = 1;
                }
                Lora_msg_result = MSG_NO_ACK;
                break;
        case MSG_NONE:
        default:
                if (!cnt)
                {
                        break;
                }
                if (fn)
                {
                        if (cmd == AT_CMD_NONE && lend[1] == '\n' && cnt)
                        {
                                *done = MSG_RECIEVE;
                        }
                        else if (cmd != AT_CMD_GET_RSSI)
                        {
                                *done = 1;
                        }
                        fn(buf, atcbpr);
                }
                break;
        }
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_MacHandle
 *Description  : Handles MAC layer operations for the LoRa module.
 *Input        : void* data - Pointer to the data for MAC layer processing.
 *Output       : None
 *-----------------------------------------------------------------------------*/

static void Lora_MacHandle(void *data)
{
        if (data != NULL)
        {
                const char *mac_prefix = "AT+MAC=0x";
                char *found = strstr((char *)data, mac_prefix);
                if (found)
                {
                        found += strlen(mac_prefix);
                        if (strlen(found) >= 8) // Ensure there are at least 8 hex digits
                        {
                                char hex_str[9]; // 8 hex digits + null terminator
                                strncpy(hex_str, found, 8);
                                hex_str[8] = '\0'; // Null-terminate the string
                                uint32_t hex_value = (uint32_t)strtoul(hex_str, NULL, 16);

                                // Copy the 4-byte hex value to Lora_mac_addr
                                g_userData.SystemData.Mac = hex_value;
                        }
                }
        }
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_get_PainID
 *Description  : Retrieves the Pain ID from the provided data.
 *Input        : void* data - Pointer to the data containing the Pain ID.
 *               void* param - Additional parameters for the operation.
 *Output       : None
 *-----------------------------------------------------------------------------*/

static void Lora_get_PainID(void *data, void *param)
{
        if (strstr((uint8_t *)data, "AT+PANID="))
        {
                char hex_str[5]; // 4 hex digits + null terminator
                strncpy(hex_str, (char *)data + strlen("AT+PANID=0x"), 4);
                hex_str[4] = '\0'; // Null-terminate the string
                uint16_t hex_value = (uint32_t)strtoul(hex_str, NULL, 16);

                Lora_painID = hex_value;
        }
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_at_init
 *Description  : Initializes the LoRa AT command interface.
 *Input        : None
 *Output       : None
 *-----------------------------------------------------------------------------*/

void Lora_at_init(void)
{
        System_DelayMs(1500); // Make sure module is stable
        uart_drv_buffer_clear();
        Lora_at_cmdSync(AT_CMD_AT, NULL, NULL, SAVE, 0, NULL, NULL, 0);
        if (UserData_getPointer()->SystemData.Provision_state == PROVISION_SUCCESS)
        {
                Lora_ChangMode(Unicast);
                Lora_SetDstAddr(g_userData.SystemData.Dst_addr);
                Lora_SetChannel(UserData_getPointer()->SystemData.ChannelID);
                Lora_SetPainID(UserData_getPointer()->SystemData.PainID);
                Lora_configCondition();
        }
        else
        {
                Lora_at_cmdSync(AT_CMD_QUERY_MAC, NULL, NULL, SAVE, 0, Lora_MacHandle, NULL, 0);
                if (g_userData.SystemData.Mac == 0xffffffff)
                {
                        Lora_at_cmdSync(AT_CMD_QUERY_MAC, NULL, NULL, SAVE, 0, Lora_MacHandle, NULL, 0);
                }
                // Lora_at_cmdSync(AT_CMD_PAINID_GET,NULL, NULL, SAVE,0, Lora_get_PainID, NULL, 0);
        }
        Lora_at_cmdSync(AT_CMD_SET_GR_ADDR, NULL, MODEL_ID, SAVE, 0, NULL, NULL, 0);
        Lora_SetSrcAddr();
        uart_drv_buffer_clear();
}

void Lora_cbfn_parseMess(uint8_t *data, Data_revc_t *param)
{
        parseMessageBuffer((uint8_t *)data, (Data_revc_t *)param);
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_Main
 *Description  : Main function for the LoRa module operations.
 *Input        : None
 *Output       : None
 *-----------------------------------------------------------------------------*/

void Lora_Main(void)
{
        memset(&Lora_data_revc, 0, sizeof(Data_revc_t));
        Lora_at_rxProcess(Lora_cbfn_parseMess, &Lora_data_revc, (uint8_t *)&Lora_msg_result, AT_CMD_NONE);
        switch (Lora_msg_result)
        {
        case MSG_NONE:
                LOG("Received result in Lora module: MSG_NONE\r\n");
                break;
        case MSG_READY:
                LOG("Received result in Lora module: MSG_READY\r\n");
                break;
        case MSG_BUSY:
                LOG("Received result in Lora module: MSG_BUSY\r\n");
                break;
        case MSG_OK:
                LOG("Received result in Lora module: MSG_OK\r\n");
                break;
        case MSG_ERROR:
                LOG("Received result in Lora module: MSG_ERROR\r\n");
                break;
        case MSG_NO_ROUTE:
                LOG("Received result in Lora module: MSG_NO_ROUTE\r\n");
                break;
        case MSG_NO_ACK:
                LOG("Received result in Lora module: MSG_NO_ACK\r\n");
                break;
        case MSG_SEND_OK:
                LOG("Received result in Lora module: MSG_SEND_OK\r\n");
                break;
        case MSG_SEND_FAIL:
                LOG("Received result in Lora module: MSG_SEND_FAIL\r\n");
                break;
        case MSG_SUCCESS:
                LOG("Received result in Lora module: MSG_SUCCESS\r\n");
                break;
        case MSG_LINE_BREAK:
                LOG("Received result in Lora module: MSG_LINE_BREAK\r\n");
                break;
        case MSG_ROUTER_READ:
                LOG("Received result in Lora module: MSG_ROUTER_READ\r\n");
                break;
        case MSG_RECIEVE:
                LOG("Received result in Lora module: MSG_RECIEVE\r\n");
                break;
        case MSG_RSSI:
                LOG("Received result in Lora module: MSG_RSSI\r\n");
                break;
        default:
                LOGF("Received result in Lora module: Unknown (%d)\r\n", Lora_msg_result);
                break;
        }

        // This part is to determine the action based on the received message result
        // If the message is MSG_RECIEVE, it means we have received a message from the gateway
        // We will process the message and update the destination address accordingly
        // If the message is MSG_SUCCESS, it means the node has connected successfully
        // If the message is MSG_NO_ROUTE, it means there is no route to the destination, we need to check the routing table
        // If the message is MSG_NO_ACK, it means we did not receive an acknowledgment for our sent message
        switch (Lora_msg_result)
        {
        case MSG_NO_ACK:
                break;
        case MSG_SUCCESS:
                LOG("Node connect successfully\r\n");
                break;
        case MSG_NO_ROUTE:
                LOG("No route to destination, check routing table\r\n");
                break;
        case MSG_RECIEVE:
                LOGF("Received LoRa msg from 0x%04X, type = 0x%02X\r\n", Lora_data_revc.src_address, Lora_data_revc.Option_type);
                Lora_msg_result = MSG_NONE;
                if ((Lora_data_revc.Option_type == 0XC3) || (g_userData.SystemData.Provision_state != PROVISION_NONE && Lora_data_revc.src_address != g_userData.SystemData.Dst_addr))
                {
                        LOGF("Received Data will be ignored because: %s\r\n", (Lora_data_revc.Option_type == 0XC3) ? ("The option type is 0XC3") : ("The source address is not coming from Gateway"));
                        break;
                }
                LOG("Node received data from Gateway\r\n");
                // This function will parse the received message and update the Lora_data_revc structure
                Lora_Msg2Data(Lora_data_revc.dst_address, &Lora_data_revc.content);
                // This function will set the Destination to the source address of the received message
                Lora_SetDstAddr(Lora_data_revc.src_address);
                break;
        default:
                break;
        }
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_GetMacAddress
 *Description  : Retrieves the MAC address of the LoRa module.
 *Input        : uint32_t macAddr - Pointer to the buffer to store the MAC address.
 *Output       : None
 *-----------------------------------------------------------------------------*/

void Lora_GetMacAddress(uint32_t *macAddr)
{
        // Lora_at_cmdSync(AT_CMD_QUERY_MAC,NULL ,NULL, SAVE, 0, Lora_MacHandle, NULL, 0);
        if (macAddr)
        {
                //                *macAddr = g_userData.SystemData.Mac;
                memcpy(macAddr, &g_userData.SystemData.Mac, 4);
        }
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_ChangeBaudRate
 *Description  : Changes the baud rate of the LoRa module.
 *Input        : uint32_t newbaud - The new baud rate to be set.
 *Output       : uint8_t - Status of the baud rate change operation.
 *-----------------------------------------------------------------------------*/

uint8_t Lora_ChangeBaudRate(uint32_t newbaud)
{
        if (Lora_at_cmdSync(AT_CMD_UART_SET, NULL, (uint8_t *)&newbaud, SAVE, sizeof newbaud, NULL, NULL, 0))
        {
                return 1;
        }
        else
        {
                return 0;
        }
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: parseMessageBuffer
 *Description  :
 *Input        :
 *Output       :
 *-----------------------------------------------------------------------------*/
/**
 * @brief Parses a message buffer and populates a Frame structure.
 *
 * This function extracts various fields from the provided buffer and assigns
 * them to the corresponding members of the Frame structure.
 *
 * @param buffer Pointer to the input buffer containing the message data.
 * @param frame Pointer to the Frame structure to be populated.
 *
 * The buffer is expected to have the following format:
 * - Byte 0: Frame type
 * - Byte 1: Data length
 * - Bytes 2-3: Network ID (2 bytes, high byte first)
 * - Bytes 4-5: Initial address (2 bytes, high byte first)
 * - Bytes 6-7: Target address (2 bytes, high byte first)
 * - Bytes 8 onwards: User data (length specified by data length field)
 */
void parseMessageBuffer(const uint8_t *buffer, Data_revc_t *frame)
{
        LOG("Received message buffer:\r\n");
        // Extract each field from the buffer
        frame->Option_type = buffer[0];
        LOGF("Option type = 0x%02X\r\n", frame->Option_type);
        frame->content_length = buffer[1];
        LOGF("Content length = %d\r\n", frame->content_length);
        frame->PainID = (buffer[3] << 8) | buffer[2]; // Combine high and low bytes
        LOGF("PainID = 0x%04X\r\n", frame->PainID);
        frame->src_address = buffer[4] | (buffer[5] << 8);
        LOGF("Source address = 0x%04X\r\n", frame->src_address);
        frame->dst_address = buffer[6] | (buffer[7] << 8);
        LOGF("Destination address = 0x%04X\r\n", frame->dst_address);
        if (frame->content_length <= MAX_content_SIZE)
        {
                if (frame->content_length)
                        memcpy(frame->content, &buffer[8], frame->content_length); // Copy user data
        }
        else
        {
                // Handle error: data length exceeds buffer size
                frame->content_length = 0; // Set data length to 0 to indicate error
        }
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_ChangMode
 *Description  : Changes the mode of Lora module.
 *Input        : eLora_option mode - The new mode to be set.
 *Output       : None.
 *-----------------------------------------------------------------------------*/
void Lora_ChangMode(eLora_option mode)
{
        Lora_at_cmdSync(AT_CMD_OPTION_SET, NULL, mode, SAVE, 0, NULL, NULL, 0);
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_SetPainID
 *Description  : Changes the PainID of Lora module.
 *Input        : uint16_t painID.
 *Output       : None.
 *-----------------------------------------------------------------------------*/
void Lora_SetPainID(uint16_t painID)
{
        Lora_at_cmdSync(AT_CMD_PAINID_SET, NULL, painID, SAVE, 0, NULL, NULL, 0);
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_SetPainID
 *Description  : Changes the channel of Lora module.
 *Input        : uint16_t channel.
 *Output       : None.
 *-----------------------------------------------------------------------------*/
void Lora_SetChannel(uint16_t channel)
{
        Lora_at_cmdSync(AT_CMD_SET_CHANNEL, NULL, channel, SAVE, 0, NULL, NULL, 0);
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_SetSrcAddr
 *Description  : Changes the source address of lora module.
 *Input        : None.
 *Output       : None.
 *-----------------------------------------------------------------------------*/
void Lora_SetSrcAddr(void)
{
        g_userData.SystemData.Src_addr = g_userData.SystemData.Mac & 0x7FFF; // Mask 15 LSB
        Lora_at_cmdSync(AT_CMD_SET_SRCADDR, NULL, g_userData.SystemData.Src_addr, SAVE, 0, NULL, NULL, 0);
        LOGF("MAC = 0x%08X, NodeID (Src_addr) = 0x%04X\r\n",
             g_userData.SystemData.Mac,
             g_userData.SystemData.Src_addr);
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_SetSrcAddr
 *Description  : Changes the destination address of lora module.
 *Input        : uint16_t Lora_dst_addr_set.
 *Output       : None.
 *-----------------------------------------------------------------------------*/
void Lora_SetDstAddr(uint16_t Lora_dst_addr_set)
{
        if (Lora_dst_addr_set != g_userData.SystemData.Dst_addr && g_userData.SystemData.Provision_state != PROVISION_SUCCESS)
        {
                g_userData.SystemData.Dst_addr = Lora_dst_addr_set;
                Lora_at_cmdSync(AT_CMD_SET_DSTADDR, NULL, g_userData.SystemData.Dst_addr, SAVE, 0, NULL, NULL, 0);
                // UserData_save();
        }
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_SendMsg
 *Description  : Send message data from lora.
 *Input        : uint8_t* data, uint32_t size.
 *Output       : None.
 *-----------------------------------------------------------------------------*/
void Lora_SendMsg(uint8_t *data, uint32_t size)
{
        Lora_msg_result = MSG_NONE;
        Lora_at_cmdSync(AT_CMD_SEND_MESSAGE, data, NULL, SAVE, size, NULL, NULL, 3500);
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_GetMsgResult
 *Description  : Return the result of message recieved.
 *Input        : None.
 *Output       : Lora_msg_result.
 *-----------------------------------------------------------------------------*/
enum Lora_message_t *Lora_GetMsgResult(void)
{
        return &Lora_msg_result;
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_SetMsgResult
 *Description  : Reset the result of message recieved.
 *Input        : Lora_message_t message.
 *Output       : None.
 *-----------------------------------------------------------------------------*/
void Lora_SetMsgResult(enum Lora_message_t message)
{
        Lora_msg_result = message;
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_GetRSSI_cb
 *Description  : The callback function handle RSSI get function.
 *Input        : void* data.
 *Output       : None.
 *-----------------------------------------------------------------------------*/
static void Lora_GetRSSI_cb(void *data)
{
        // uint16_t chars_read = 0;
        // uint8_t vTemp = vRouting.NoRouter;
        // sscanf((char*)data, "%d %d %d %d %d",
        //                    &vRouting.NodeRoute[vTemp].No,
        //                    &vRouting.NodeRoute[vTemp].DestAdd,
        //                    &vRouting.NodeRoute[vTemp].TarAdd,
        //                    &vRouting.NodeRoute[vTemp].Sc,
        //                    &vRouting.NodeRoute[vTemp].Rssi);
        // vRouting.NoRouter++;
        // data += chars_read;
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_GetRSSI
 *Description  : Get RSSI of node (inside network routing).
 *Input        : None.
 *Output       : int8_t RSSI .
 *-----------------------------------------------------------------------------*/
int8_t Lora_GetRSSI(void)
{
        // memset(&vRouting,0,sizeof(vRouting));
        // Lora_at_cmdSync(AT_CMD_GET_RSSI,NULL, NULL, SAVE, 0, Lora_GetRSSI_cb, NULL, 500);
        // for(uint16_t i=0;i< vRouting.NoRouter; i++)
        // {
        // 	if(vRouting.NodeRoute[i].DestAdd == g_userData.SystemData.Dst_addr)
        // 	{
        // 		RSSI = (int8_t)vRouting.NodeRoute[i].Rssi;
        // 		break;
        // 	}
        // }
        return RSSI;
}

/*------------------------------------------------------------------------------
 *Engineer     : HuyDoan
 *Historical   : 1. November 19, 2024
 *Function name: Lora_configCondition
 *Description  : Init the Lora module condition by user.
 *Input        : None.
 *Output       : None.
 *-----------------------------------------------------------------------------*/
void Lora_configCondition(void)
{
        strNode_config *LoraConfig = &UserData_getPointer()->NodeConfig;
        Lora_at_cmdSync(AT_CMD_POWER_SET, NULL, LoraConfig->max_tx_power, SAVE, 0, NULL, NULL, 0);
        Lora_at_cmdSync(AT_CMD_CSMA_RNG, NULL, LoraConfig->CSMA_time, 0, 0, NULL, NULL, 0);
        Lora_at_cmdSync(AT_CMD_RESET_TIME, NULL, LoraConfig->reset_rf_time, 0, 0, NULL, NULL, 0);
        Lora_at_cmdSync(AT_CMD_ROUTER_SCORE, NULL, LoraConfig->router_score, 0, 0, NULL, NULL, 0);
        Lora_at_cmdSync(AT_CMD_MEMBER_RAD, NULL, LoraConfig->propagation_radius, SAVE, 0, NULL, NULL, 0);
        Lora_at_cmdSync(AT_CMD_NONMEMBER_RAD, NULL, LoraConfig->propagation_radius, SAVE, 0, NULL, NULL, 0);
}
