/*
  * LCD_Dwin.c
  *
  *  Created on: 25 Nov 2024
  *      Author: ADMIN
  */
 
 
 //==============================================================================
 //---------------------------------Includes-------------------------------------
 //==============================================================================
 #include "LCD_Dwin.h"
 #include <string.h>
 #include <stdio.h>
 #include <stdlib.h>

#if LCD_CONTROLER

 //==============================================================================
 //---------------------------------Defines--------------------------------------
 //==============================================================================
 
 #define headerDWIN_H 0x5A
 #define headerDWIN_L 0xA5
 #define DWIN_UART_DELAY 3000
 //==============================================================================
 //--------------------------------Variables-------------------------------------
 //==============================================================================
  struct readDataDWIN_P readDataDWIN;
 
 //==============================================================================
 //--------------------------------PROTOTYPE-------------------------------------
 //==============================================================================
 uint8_t getByte(unsigned char*b);
 int test;
 uint16_t m=0;
 uint16_t bbb;
 uint8_t ccc[9];
 static uint8_t Lcd_send_buffer[30];
 volatile uint8_t blSend = 0;
 static timer_t Remain_timerLCD;
 //==============================================================================
 //--------------------------Device initialization-------------------------------
 //==============================================================================
 void LCD_DWIN_Init(void)
 {
   LCD_on_init();
   Lcd_uart_init();
   Sensor_t *Sensor = Sensor_get_value();
   LCD_power(Sensor->LCD_state);
   LCD_Refresh_remainTime();
   readDataDWIN.delayOverflowMaxTime = DWIN_UART_DELAY;
   for (uint16_t i = 0; i <DWIN_UART_BUFFER_SIZE;i++){readDataDWIN.uartBuffer[i] = 0xff;}
   if(UserData_getPointer()->SystemData.Provision_state == PROVISION_SUCCESS)
   {
	   LCD_initScreen();
   }
   else
   {
    System_DelayMs(1500);
      Proc_DWIN(GOTO_PAGE, 0,0, 0,0x01);
   }
 }
 //==============================================================================
 //--------------------------------FUNCTIONS-------------------------------------
 //==============================================================================
 
 //==============================================================================
 //---------------------------preset variable DWIN-------------------------------
 //==============================================================================
 
 //==============================================================================
 //--------------------------Retrieving_a_data_byte------------------------------
 //==============================================================================
 uint8_t getByte(unsigned char*b)
 {
   if (readDataDWIN.uartBuffer[readDataDWIN.uartCnt]!=0xff)
   {
     *b=(readDataDWIN.uartBuffer[readDataDWIN.uartCnt] & 0xff);
     readDataDWIN.uartBuffer[readDataDWIN.uartCnt]=0xff;
     readDataDWIN.uartCnt++;
     if (readDataDWIN.uartCnt == DWIN_UART_BUFFER_SIZE) readDataDWIN.uartCnt=0;
     return 1;
   }
   else
   {
    return 0;
   }
 }
 
 void receivingDataPacket(void)
 {
   uint16_t m = 0;
   if (!readDataDWIN.PacketReady)
   {
	 getByte(&readDataDWIN.GettingByte);
	 m = 0;
	   if (readDataDWIN.GettingByte == 0x5A)
	   {
		 readDataDWIN.rxData[m] = readDataDWIN.GettingByte; // start byte 1
		 m++;
		 readDataDWIN.timeoutDelay = 0;
		 getByte(&readDataDWIN.GettingByte);
		 readDataDWIN.rxData[m] = readDataDWIN.GettingByte; // start byte 2
		 m++;
		 readDataDWIN.timeoutDelay = 0;
		 getByte(&readDataDWIN.GettingByte);
		 readDataDWIN.rxData[m] = readDataDWIN.GettingByte; // Data packet length
		 readDataDWIN.lengthRxPacket = readDataDWIN.rxData[m];
		 m++;
		 readDataDWIN.timeoutDelay = 0;
		 for(uint16_t i = 0;i < readDataDWIN.lengthRxPacket; i++) // Fill the buffer with data
		 {
		   getByte(&readDataDWIN.GettingByte);
		   readDataDWIN.timeoutDelay = 0;
		   readDataDWIN.rxData[m] = readDataDWIN.GettingByte; // Retrieving data
		   m++;
		 }
	   readDataDWIN.PacketReady = true;
	   }
	 readDataDWIN.GettingByte = 0;
   }
 }
 
 uint8_t parsingDWIN(void)
 {
   receivingDataPacket();
   if (readDataDWIN.PacketReady)
   {
     readDataDWIN.parsingDataDWIN.header = (uint16_t)readDataDWIN.rxData[0]<<8 | (uint16_t)readDataDWIN.rxData[1];
     readDataDWIN.parsingDataDWIN.length = readDataDWIN.rxData[2];
     readDataDWIN.parsingDataDWIN.command = (enum_Instruction_Dwin)readDataDWIN.rxData[3];
     for (uint8_t i = 0; i < (readDataDWIN.lengthRxPacket - 1);i++)
     {
       readDataDWIN.parsingDataDWIN.data[i] = readDataDWIN.rxData[i+4];
     }
     return true;
   }

 }
 //==============================================================================
 //----------------------------DWIN GO TO PAGE-----------------------------------
 //==============================================================================
 void goToPageDWIN(uint16_t page)
 {
   uint8_t str[10] = {headerDWIN_H,headerDWIN_L,0x07,0x82,0x00,0x84,0x5A,0x01,(page & 0xFF00)>>8,page & 0xFF};
 
   LCD_Sendcmd( str, sizeof(str));
 }
 
 
 uint8_t* HMI_getRxBuff(void)
 {
     return readDataDWIN.uartBuffer;
 }
 
 static void DWIN_getCmd( Dwin_Proc proc, enum_LCDRegisterAddress_t address, uint8_t* p_data,uint32_t data, uint16_t *len, uint16_t pram)
 {
 
     switch (proc)
     {
     case GOTO_PAGE:
         *len = 10;
         uint8_t temp_str_goto_page[10] = {headerDWIN_H, headerDWIN_L, 0x07, WRITE_VAR_INS, 0x00, 0x84, 0x5A, 0x01, (pram & 0xFF00) >> 8, pram & 0xFF};
         memcpy(Lcd_send_buffer, temp_str_goto_page, *len);
         break;
     case WRITE_LCD_2BYTES:
         *len = 8;
         uint8_t temp_str_write[8] = {headerDWIN_H, headerDWIN_L, 0x05, WRITE_VAR_INS, (address & 0xFF00) >> 8, address & 0xFF, (data & 0xFF00) >> 8, data & 0xFF};
         memcpy(Lcd_send_buffer, temp_str_write, *len);
         break;
     case WRITE_LCD_4BYTES:
         *len = 10;
         uint8_t temp_str_write_2bytes[10] = {headerDWIN_H, headerDWIN_L, 0x07, WRITE_VAR_INS, (address & 0xFF00) >> 8, address & 0xFF, (data & 0xFF000000) >> 24, (data & 0xFF0000) >> 16, (data & 0xFF00) >> 8, data & 0xFF};
         memcpy(Lcd_send_buffer, temp_str_write_2bytes, *len);
         break;
     case READ_LCD_1BYTE:
         *len = 7;
         uint8_t temp_str_read[7] = {headerDWIN_H, headerDWIN_L, 0x04, READ_VAR_INS, (address & 0xFF00) >> 8, address & 0xFF, pram};
         memcpy(Lcd_send_buffer, temp_str_read, *len);
         break;
      case WRITE_LCD_MULTI_BYTES:
          *len = 30;
          uint8_t temp_str_write_multi_bytes[6] = {headerDWIN_H, headerDWIN_L, 27, WRITE_VAR_INS, (address & 0xFF00) >> 8, address & 0xFF};
          memcpy(Lcd_send_buffer, temp_str_write_multi_bytes, 6);
          memcpy(Lcd_send_buffer + 6, p_data, 24);
          break;
      case  WRITE_LCD_INIT_INFO:
          *len = 24;
          uint8_t temp_str_write_init_info[6] = {headerDWIN_H, headerDWIN_L, 21, WRITE_VAR_INS, (address & 0xFF00) >> 8, address & 0xFF};
          memcpy(Lcd_send_buffer, temp_str_write_init_info, 6);
          memcpy(Lcd_send_buffer + 6, p_data, 18);
          break;
      case WRITE_RELAY_BUTTON:
          uint8_t temp_str_write_relay[12] = {headerDWIN_H, headerDWIN_L, 0x09, WRITE_VAR_INS, (address & 0xFF00) >> 8, address & 0xFF,0 ,*p_data ,0 ,*(p_data+1) ,0 ,*(p_data+2)};
          memcpy(Lcd_send_buffer, temp_str_write_relay, 12);
          *len = sizeof(temp_str_write_relay);
        break;
     case READ_LCD_2BYTES:
     default:
         return; // Return if the proc is not recognized
     }
   
 }
 
 bool Proc_DWIN(Dwin_Proc proc, enum_LCDRegisterAddress_t address, uint8_t* p_data, uint32_t data, uint16_t pram)
 {

	blSend = 1;
    uint16_t len = 0;
    DWIN_getCmd(proc, address,p_data,data,&len,pram);
 
    LCD_Sendcmd( Lcd_send_buffer, len);
    System_DelayMs(5);
 }
 
 void Dwin_Parse_readData(enum_LCDRegisterAddress_t address, uint16_t pram)
 {
	 Sensor_t *Sensor = Sensor_get_value();
     switch (address)
     {
       case LCD_RELAY_1:
    	    Sensor->Relay[0] = pram;
    	    Proc_Change_ProcCmd(PROC_RELAY, CMD_RELAY);
          Proc_relay_refreshTimer();
          Sensor->Relay_isChange = RELAY_CHANGE_BY_LCD;
           break;
       case LCD_RELAY_2:
          Sensor->Relay[1] = pram;
          Proc_Change_ProcCmd(PROC_RELAY, CMD_RELAY);
          Proc_relay_refreshTimer();
          Sensor->Relay_isChange = RELAY_CHANGE_BY_LCD;
           break;
       case LCD_RELAY_3:
          Sensor->Relay[2] = pram;
    	    Proc_Change_ProcCmd(PROC_RELAY, CMD_RELAY);
          Proc_relay_refreshTimer();
          Sensor->Relay_isChange = RELAY_CHANGE_BY_LCD;
          break;
       case LCD_OVER_VOLTAGE:
           break;
       case LCD_OVER_CURRENT:
           break;
       case LCD_OVER_POWER:
           break;
       case LCD_REFERESH_DATA:
           break;
       case LCD_TURN_OFF_LCD:
            Sensor->LCD_state = pram;
            LCD_power(Sensor->LCD_state);
           break;
     default:
       break;
     }
 }
 
 void LCD_parse(void)
 {
  uint8_t result = 0;
      if(readDataDWIN.PacketReady)
      {
          readDataDWIN.PacketReady = false;
          enum_LCDRegisterAddress_t address = (readDataDWIN.parsingDataDWIN.data[0])<<8|readDataDWIN.parsingDataDWIN.data[1];
          uint16_t data = (readDataDWIN.parsingDataDWIN.data[3])<<8|readDataDWIN.parsingDataDWIN.data[4];
          switch(readDataDWIN.parsingDataDWIN.command)
          {
              case NONE_INS:
                  break;
              case WRITEREG_INS:
                  break;
              case READREG_INS:
                  break;
              case WRITE_VAR_INS:
                  // if((uint16_t)((readDataDWIN.pars ingDataDWIN.data[0]<<8) & readDataDWIN.parsingDataDWIN.data[1])== DWIN_OK_MESSAGE)
                  // {
                  //     result = true;
                  // }
                  break;
              case READ_VAR_INS:
                  LCD_Refresh_remainTime();
                  Dwin_Parse_readData(address,data);
                  break;
              default:
                  break;
          }
      }
 }
 void LCD_Refresh_remainTime(void)
 {
     Timer_Create(&Remain_timerLCD,LCD_REMAIN_TIME);
 }
 void LCD_RemainTime_check(void)
 {
	 Sensor_t *Sensor = Sensor_get_value();
     if(Timer_Timeout(&Remain_timerLCD)&& Sensor->LCD_state ==1 )
     {

        Sensor->LCD_state = 0;
        LCD_power(Sensor->LCD_state);
     }
 }
 void LCD_initScreen(void)
 {
	 System_DelayMs(1500);
    if(UserData_getPointer()->SystemData.Provision_state == PROVISION_SUCCESS)
   { 
	    Proc_DWIN(GOTO_PAGE, 0,0, 0,0x0);
   }
   else
   {
      System_DelayMs(1500);
      Proc_DWIN(GOTO_PAGE, 0,0, 0,0x01);
   }
	 uint8_t LCDtemp_buff[18] = {0, UserData_getPointer()->SystemData.Fw_ver_major, 0, UserData_getPointer()->SystemData.Fw_ver_minor, 0, UserData_getPointer()->SystemData.Fw_ver_path,\
	(UserData_getPointer()->SystemData.Mac&0xFF000000)>>24,\
	(UserData_getPointer()->SystemData.Mac&0x00FF0000)>>16,\
	(UserData_getPointer()->SystemData.Mac&0x0000FF00)>>8,\
	(UserData_getPointer()->SystemData.Mac&0xFF),\
	((int)UserData_getPointer()->Node_userConfig.latitude&0xFF000000)>>24,\
	((int)UserData_getPointer()->Node_userConfig.latitude&0x00FF0000)>>16,\
	((int)UserData_getPointer()->Node_userConfig.latitude&0x0000FF00)>>8,\
	((int)UserData_getPointer()->Node_userConfig.latitude&0x000000FF),\
	((int)UserData_getPointer()->Node_userConfig.longitude&0xFF000000)>>24,\
	((int)UserData_getPointer()->Node_userConfig.longitude&0x00FF0000)>>16,\
	((int)UserData_getPointer()->Node_userConfig.longitude&0x0000FF00)>>8,\
	((int)UserData_getPointer()->Node_userConfig.longitude&0x000000FF)};
	Proc_DWIN(WRITE_LCD_INIT_INFO, LCD_VERSIOM_MAJOR,(uint8_t*)LCDtemp_buff, 0,0x01);
 }
 #endif
 //==============================================================================
 //---------------------------------END FILE-------------------------------------
 //==============================================================================
