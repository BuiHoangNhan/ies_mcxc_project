/*
  * LCD_Dwin.h
  *
  *  Created on: 25 Nov 2024
  *      Author: HuyDQ
  */

#ifndef __LCD_LCD_DWIN_H_
#define __LCD_LCD_DWIN_H_



//==============================================================================
//---------------------------------Includes-------------------------------------
//==============================================================================

#include <stdbool.h>
#include "stdint.h"
#include <1_Hardware/2_LCD/Uart_LCD.h>
#include <1_Hardware/0_Timer/timer.h>
#include <2_Middle/4_Sensor/Sensor.h>
#include <3_application/Sys_Process.h>
//==============================================================================
//---------------------------------Defines--------------------------------------
//==============================================================================


#define DWIN_UART_BUFFER_SIZE 9
#define DWIN_OK_MESSAGE 0x4f4b
//==============================================================================
//--------------------------------Variables-------------------------------------
//==============================================================================

typedef enum __enum_LCDRegisterAddress_t
{
  LCD_VOLTAGE_PHASE1 =  0x1001,
  LCD_CURRENT_PHASE1 =  0x1002,
  LCD_POWER_PHASE1   =  0x1003,

  LCD_VOLTAGE_PHASE2 =  0x1004,
  LCD_CURRENT_PHASE2 =  0x1005,
  LCD_POWER_PHASE2   =  0x1006, 

  LCD_VOLTAGE_PHASE3 =  0x1007,
  LCD_CURRENT_PHASE3 =  0x1008,
  LCD_POWER_PHASE3   =  0x1009, 

  LCD_RSSI           =  0x100D,
  LCD_VERSIOM_MAJOR  =  0x2001,
  LCD_VERSIOM_MINOR  =  0x2002,
  LCD_VERSIOM_PATCH	 =  0x2003,
  LCD_ID_MAC         =	0x2004,
  LCD_LATITUDE       =	0x2006,
  LCD_LONGTITUDE     =	0x2008,

  LCD_RELAY_1        =	0x3001,
  LCD_RELAY_2        =	0x3002,
  LCD_RELAY_3        =	0x3003,

  LCD_OVER_VOLTAGE   =  0x3004,
  LCD_OVER_CURRENT   =	0x3005,
  LCD_OVER_POWER     =	0x3006,
	
  LCD_REFERESH_DATA  =	0x4001,
  LCD_TURN_OFF_LCD   =	0x4002,
    
} enum_LCDRegisterAddress_t;

typedef enum
{
   NONE_INS = 0x00,
   WRITEREG_INS = 0x80,
   READREG_INS = 0x81,
   WRITE_VAR_INS = 0x82,
   READ_VAR_INS = 0x83
} enum_Instruction_Dwin;

typedef enum __enum_Dwin_Proc
{
   GOTO_PAGE = 0x00,
   WRITE_LCD_2BYTES = 0x01,
   WRITE_LCD_4BYTES = 0x02,
   READ_LCD_1BYTE = 0x03,
   READ_LCD_2BYTES = 0x04,
   WRITE_LCD_MULTI_BYTES = 0x05,
} Dwin_Proc;

typedef struct
{
   enum_LCDRegisterAddress_t address;
   uint16_t data;
} Data_address_t;

struct parsingDataDWIN_P
{
   uint16_t header;
   uint8_t length;
   enum_Instruction_Dwin command;
   uint8_t data[12];
};
#pragma pack(1)
struct readDataDWIN_P{
      volatile uint8_t uartBuffer[DWIN_UART_BUFFER_SIZE];
      uint16_t uartCnt;
      uint8_t GettingByte;
      volatile bool PacketReady;
      uint16_t delayOverflowMaxTime;
      uint16_t lengthRxPacket;
      bool timeoutOverflow;
      uint8_t rxData[9];
      uint16_t timeoutDelay;
      struct parsingDataDWIN_P parsingDataDWIN;
   } ;


   typedef struct __struct_LCD_Data
   {
    uint16_t U1;
    uint16_t I1;
    uint32_t P1;
    uint16_t U2;
    uint16_t I2;
    uint32_t P2;
    uint16_t U3;
    uint16_t I3;
    uint32_t P3;
   } struct_LCD_Data;

   #pragma pack()
//==============================================================================
//--------------------------------PROTOTYPE-------------------------------------
//==============================================================================


//==============================================================================
//--------------------------------FUNCTIONS-------------------------------------
//==============================================================================
void dwinUartDmaInit();
Data_address_t  value_of_readDataDwin(void);
bool Proc_DWIN(Dwin_Proc proc, enum_LCDRegisterAddress_t address, uint8_t* p_data, uint32_t data, uint16_t pram);
enum_Instruction_Dwin Dwin_get_curCMD(void);
uint8_t parsingDWIN(void);
void LCD_parse(void);
//==============================================================================
//---------------------------------END FILE-------------------------------------
//==============================================================================

#endif /* __2_LCD_LCD_DWIN_H_ */
