/**
* @file user_data.h
* @brief 
*
* @details 
* @date 2024 Nov 28
* @version 1
* @author Duong Quoc Hoang
* Group: 
*
*/

#ifndef USER_DATA_H_
#define USER_DATA_H_

#include <stdint.h>
#include <stdbool.h>
#include <3_Application/Sys_Process.h>
#ifdef __cplusplus
extern "C" {
#endif

#define USER_DATA_MAGIC_SIZE (2)
#define USER_DATA_MAGIC_CODE {0xaa, 0xaa}

#define DATA_PERIOD_CONFIG_DEFAULT 10
#define MAX_TX_POWER_CONFIG_DEFAULT 10

	typedef enum __enum_Provision_state
	{
		PROVISION_NONE = 0xFF,
		PROVISION_PENDING = 0x55,
		PROVISION_SUCCESS = 0xAA,
	} enum_Provision_state;

    #pragma pack(1)
    typedef struct __strNode_config
    {
        uint16_t max_tx_power;
        uint8_t reset_rf_time;
        uint8_t propagation_radius;
        uint16_t CSMA_time;
        uint16_t router_score;
        uint16_t data_period;
    } strNode_config;
    
    typedef struct __strtNode_userConfig
    {
        uint16_t vol_thres;
        uint16_t current_thres;
#if NODE_NEMA
        uint16_t power_thres;
        uint16_t temperature_thres; 
#elif LCD_CONTROLER
        uint32_t power_thres;    
        uint32_t latitude;
        uint32_t longitude;
#endif
    } strtNode_userConfig;

	typedef struct __strSystem_data
	{
		uint32_t    Mac;
		uint16_t    Src_addr;
		uint16_t    Dst_addr;
		uint16_t    PainID;
		uint16_t    ChannelID;
		uint16_t    Model_id;
		enum_Provision_state Provision_state;
		uint8_t     Fw_ver_major;
		uint8_t     Fw_ver_minor;
		uint8_t     Fw_ver_path;
	} strSystem_data ;

#pragma pack()

#pragma pack(1)
typedef struct user_data
{
    strSystem_data      SystemData;
    strNode_config      NodeConfig;
    strtNode_userConfig Node_userConfig;
    uint8_t magic_code[USER_DATA_MAGIC_SIZE]; //validate        
    uint8_t bReserve[64-(sizeof(strSystem_data)+sizeof(strtNode_userConfig)+sizeof(strNode_config)+USER_DATA_MAGIC_SIZE)];
} UserData;
#pragma pack()
    UserData* UserData_getPointer(void);
/****************************Function prototypes*******************************/

    int UserData_init(void);
	int UserData_reload(void);
	int UserData_save(void);

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* USER_DATA_H_ */

/* End of user_data.h */
