#ifndef __SYS_PROCESS_H
#define __SYS_PROCESS_H

#include <1_Hardware/0_Timer/System_tick_timer.h>
#include <1_Hardware/3_Led_IO/IO_driver.h>
#include <2_Middle/2_LCD/LCD_Dwin.h>
#include <3_Application/3_Led_Button_Relay_Dimmer/Proc_Led_Button_Relay_Dimmer.h>
#include <2_Middle/7_ota/ota.h>
#include <2_Middle/8_user_data/user_data.h>
#include <2_Middle/4_Sensor/Sensor.h>
#include <2_Middle/9_Power_Atm90e226/atm90e26.h>
#include <3_Application/1_Proc_Lora/Proc_Lora_main.h>
#define NUMBER_STACK_PROC 5

#if LCD_CONTROLER

#define ALERT_VOLTAGE_GET      (Sensor_get_value()->BL0942_sensor_data[0].voltage > UserData_getPointer()->Node_userConfig.vol_thres) \
                                    || (Sensor_get_value()->BL0942_sensor_data[1].voltage > UserData_getPointer()->Node_userConfig.vol_thres) \
                                    || (Sensor_get_value()->BL0942_sensor_data[2].voltage > UserData_getPointer()->Node_userConfig.vol_thres)
#define ALERT_CURRENT_GET      (Sensor_get_value()->BL0942_sensor_data[0].current > UserData_getPointer()->Node_userConfig.current_thres) \
                                    || (Sensor_get_value()->BL0942_sensor_data[1].current > UserData_getPointer()->Node_userConfig.current_thres) \
                                    || (Sensor_get_value()->BL0942_sensor_data[2].current > UserData_getPointer()->Node_userConfig.current_thres)
#define ALERT_POWER_GET        (Sensor_get_value()->BL0942_sensor_data[0].power > UserData_getPointer()->Node_userConfig.power_thres) \
                                    || (Sensor_get_value()->BL0942_sensor_data[1].power > UserData_getPointer()->Node_userConfig.power_thres) \
                                    || (Sensor_get_value()->BL0942_sensor_data[2].power > UserData_getPointer()->Node_userConfig.power_thres)
#define ALERT_GET              (((ALERT_VOLTAGE_GET) ? ALERT_OVER_VOLTAGE : ALERT_NONE) \
                                    | ((ALERT_CURRENT_GET) ? ALERT_OVER_CURRENT : ALERT_NONE) \
                                    | ((ALERT_POWER_GET) ? ALERT_OVER_POWER : ALERT_NONE))

#elif NODE_NEMA

#define ALERT_VOLTAGE_GET      (Sensor_get_value()->ATM90e26_sensor_data.voltage > UserData_getPointer()->Node_userConfig.vol_thres)
#define ALERT_CURRENT_GET      (Sensor_get_value()->ATM90e26_sensor_data.current > UserData_getPointer()->Node_userConfig.current_thres)
#define ALERT_POWER_GET        (Sensor_get_value()->ATM90e26_sensor_data.power > UserData_getPointer()->Node_userConfig.power_thres)
#define ALERT_HEAT_GET         (Sensor_get_value()->Temperature > UserData_getPointer()->Node_userConfig.temperature_thres)

#define ALERT_GET              (((ALERT_VOLTAGE_GET) ? ALERT_OVER_VOLTAGE : ALERT_NONE) \
                                    | ((ALERT_CURRENT_GET) ? ALERT_OVER_CURRENT : ALERT_NONE) \
                                    | ((ALERT_POWER_GET) ? ALERT_OVER_POWER : ALERT_NONE) \
                                    | ((ALERT_HEAT_GET) ? ALERT_OVER_HEAT : ALERT_NONE))

#endif

typedef enum __enum_ProcID
{
    PROC_NONE = 0,

/*==========================================
        PROCESS BL0942
===========================================*/
    PROC_BL0942 = 1,
#if LCD_CONTROLER
/*==========================================
        PROCESS LCD
===========================================*/
    PROC_LCD_UPDATE = 2,
/*==========================================
        PROCESS RELAY
===========================================*/
    PROC_RELAY = 3,
#elif NODE_NEMA
/*==========================================
        PROCESS SENSOR
===========================================*/
    PROC_SENSOR = 4,
/*==========================================
        PROCESS DIMMER
===========================================*/
    PROC_DIMMER = 5,
#endif
/*==========================================
        PROCESS LORA
===========================================*/
    PROC_LORA = 6,   

    PROC_ID_MAX = 65535

} enum_ProcID;

typedef enum __enum_CMDID
{
    CMD_NONE = 0,


#if LCD_CONTROLER
/*==========================================
        CMD BL0942
===========================================*/    
CMD_BL0942 = 1,
/*==========================================
        CMD LCD
===========================================*/
    CMD_LCD_UPDATE_RELAY = 2 ,
    CMD_LCD_UPDATE_UIP = 3,
    CMD_LCD_UPDATE_SETUP = 4,
    CMD_LCD_UPDATE_UIPTH = 5,
/*==========================================
        CMD RELAY
===========================================*/
    CMD_RELAY = 6,
#elif NODE_NEMA
/*==========================================
		CMD ATM90E26
===========================================*/
	CMD_ATM90E26 = 1,
/*==========================================
        CMD SENSOR LIGHT
===========================================*/
    CMD_SENSOR_LIGHT = 6,
/*==========================================
        CMD SENSOR TEMP
===========================================*/
    CMD_SENSOR_TEMP = 7,
/*==========================================
        CMD LED DIM
===========================================*/
    CMD_DIMMER = 8,
#endif
/*==========================================
        CMD LORA MESSAGE
===========================================*/
    CMD_LORA_SEND_REQ_NODEID = 9,
    CMD_LORA_SEND_DATA = 10,
    CMD_LORA_SEND_ACK  = 11,
    CMD_LORA_SEND_NACK = 12,
    CMD_LORA_PROVISION = 13,
    CMD_LORA_CF_REQ_ID = 14,
    CMD_LORA_PARSE_FW  = 15,
    CMD_LORA_START_OTA = 16,
    CMD_GW_SEND_CONFIG = 17,
    CMD_ID_MAX = 65535

} enum_CMDID;

typedef struct
{
    enum_ProcID  eProc; // Type of process (PROC_LORA, ...)
	enum_CMDID   eCmd; // Type of command (CMD_LORA_SEND_DATA, CMD_LORA_SEND_ACK, ...)
} structProcHandler;

void Proc_Process(void);

void Proc_Change_ProcCmd(enum_ProcID Proc, enum_CMDID Cmd);

enum_ProcID Proc_GetProc(void);

enum_CMDID Proc_GetCmd(void);

void Proc_System_ChangProc(void);

void Driver_Init(void);

#endif //__SYS_PROCESS_H
