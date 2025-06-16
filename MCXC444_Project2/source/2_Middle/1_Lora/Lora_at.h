#ifndef __LORA_AT_H
#define __LORA_AT_H
#include <1_Hardware/0_Timer/timer.h>
#include <1_Hardware/2_LCD/Uart_LCD.h>
#include <stdint.h>
#include <stdbool.h>

#define TOK_AND_MSG(x, msg) {x, sizeof(x) - 1, msg}
#define MAX_content_SIZE 256

#define MAX_ROUTING_TABLE_SIZE             (5)


typedef void (*atcbfn) (void*, void*);

typedef enum {
  Unicast = 1,
  Multicast,
  Broadcast,
  Anyast,
} eLora_option;
#pragma pack(1)
typedef struct {
    eLora_option Option_type;
    uint8_t content_length;
    uint16_t PainID;
    uint16_t src_address;
    uint16_t dst_address;
    uint8_t content[MAX_content_SIZE]; // Max size for user data
} Data_revc_t;
#pragma pack()
typedef enum
{
  //QUERY
  AT_CMD_NONE = 0,

  AT_CMD_AT,
  AT_CMD_RESET,
  AT_CMD_DEFAULT,
  AT_CMD_POWER_GET,
  AT_CMD_UART_GET,
  AT_CMD_RATE_GET,
  AT_CMD_OPTION_GET,
  AT_CMD_PAINID_GET,
  AT_CMD_GET_SRCADDR,
  AT_CMD_GET_DSTADDR,
  AT_CMD_GET_GR_ADDR,
  AT_CMD_QUERY_MAC,
  AT_CMD_GET_RSSI,

  //SET
  AT_CMD_POWER_SET,
  AT_CMD_UART_SET,
  AT_CMD_RATE_SET,
  AT_CMD_OPTION_SET,
  AT_CMD_PAINID_SET,
  AT_CMD_SET_SRCADDR,
  AT_CMD_SET_DSTADDR,
  AT_CMD_SET_GR_ADDR,
  AT_CMD_SET_CHANNEL,
  AT_CMD_GR_DEL,
  AT_CMD_SEND_MESSAGE,

 
} eAtCmd;


typedef enum {
  CMD_STATUS_INIT = 0,
  CMD_STATUS_WAIT_RESULT,
  CMD_STATUS_DONE,
  CMD_STATUS_TIMEOUT,
} eCmdStatus;

enum Lora_message_t {
        MSG_NONE = 0,
        MSG_READY,
        MSG_BUSY,
        MSG_OK,
        MSG_ERROR,  
        MSG_NO_ROUTE,
        MSG_NO_ACK,
        MSG_SEND_OK,
        MSG_SEND_FAIL,
        MSG_SUCCESS,
        MSG_LINE_BREAK,
        MSG_ROUTER_READ,
        MSG_RECIEVE,
        MSG_RSSI,
};

typedef enum __enum_save_t{
  NO_SAVE,
  SAVE,
} enum_save_t;

#pragma pack(1)
struct rx_status_tok {
        uint8_t *tok;
        uint32_t tok_len;
        enum Lora_message_t msg;
};


#pragma pack()

typedef struct __str_LoraRouting{
    int  No;
    int DestAdd;
    int TarAdd;
    int  Sc;
    int  Rssi;
} str_LoraRouting;
typedef struct __str_NodeRoute
{
  uint16_t NoRouter;
  str_LoraRouting NodeRoute[MAX_ROUTING_TABLE_SIZE];
}str_NodeRoute;

extern Data_revc_t Lora_data_revc;
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

uint32_t Lora_at_cmdSync(eAtCmd cmd, uint8_t* message, uint32_t input, enum_save_t save_type, uint32_t input_size, atcbfn cbfn, void* atcbpr, uint32_t timeout);
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

void Lora_at_rxProcess(atcbfn fn, void* atcbpr, uint8_t* done, eAtCmd cmd);

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Lora_at_init
*Description  : Initializes the LoRa AT command interface.
*Input        : None
*Output       : None
*-----------------------------------------------------------------------------*/

void Lora_at_init(void);

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Lora_GetMacAddress
*Description  : Retrieves the MAC address of the LoRa module.
*Input        : uint32_t macAddr - Pointer to the buffer to store the MAC address.
*Output       : None
*-----------------------------------------------------------------------------*/

void Lora_GetMacAddress(uint32_t* macAddr);

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Lora_ChangeBaudRate
*Description  : Changes the baud rate of the LoRa module.
*Input        : uint32_t newbaud - The new baud rate to be set.
*Output       : uint8_t - Status of the baud rate change operation.
*-----------------------------------------------------------------------------*/

uint8_t Lora_ChangeBaudRate(uint32_t newbaud);

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: parseMessageBuffer
*Description  :
*Input        :
*Output       :
*-----------------------------------------------------------------------------*/

void parseMessageBuffer(const uint8_t *buffer, Data_revc_t *frame);

void Lora_ChangMode(eLora_option mode);

void Lora_SetPainID(uint16_t painID);

void Lora_SetChannel(uint16_t channel);

void Lora_SetSrcAddr(void);

void Lora_SetDstAddr(uint16_t Lora_dst_addr_set);

void Lora_SendMsg(uint8_t* data, uint32_t size);

void Lora_Main(void);


enum Lora_message_t * Lora_GetMsgResult(void);

int8_t Lora_GetRSSI(void);

void Lora_SetMsgResult(enum Lora_message_t message);
#endif /*__TCP_Lora_AT_H*/
