#include "Button.h"

#if LCD_CONTROLER
strt_Button Button[5];
#define Pair_button Button[4]
#elif NODE_NEMA
strt_Button Pair_button;
#endif

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Button_setState
*Description  : Set state of pressed button in callback function.
*Input        : uint8_t button_state.
*Output       : None.
*-----------------------------------------------------------------------------*/
void Button_setState(uint8_t button_state)
{
#if LCD_CONTROLER
		switch (button_state)
      {
         case GPIO_BUTTON_LEFT:
         case GPIO_BUTTON_BOTTOM:
         case GPIO_BUTTON_UP:
         case GPIO_BUTTON_RIGHT:
            Button[button_state].isPressed = true;
            break;
         case GPIO_BUTTON_MIDDLE:
            Pair_button.isPressed = true;
            Pair_button.timer_state = true;
            Timer_Create(&Pair_button.timePressed , 3000);
            break;
         default:
            break;
      }
#elif NODE_NEMA
      Pair_button.isPressed = true;
      Pair_button.timer_state = true;
      Timer_Create(&Pair_button.timePressed , 3000);
#endif
}

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Button_Handler
*Description  : In Loop to handler the button pressed.
*Input        : None.
*Output       : None.
*-----------------------------------------------------------------------------*/
void Button_Handler(void)
 {
     Pair_button.isPressed = (Pair_button_read()==0)?true:false;
     if(Timer_Timeout(&Pair_button.timePressed)&&Pair_button.isPressed == true&& Pair_button.timer_state ==true)
     {
		 LOG("Pair Button is pressed, start to provision\r\n");
        Lora_set_toProvision();
     }
     else if(Pair_button.isPressed == false&& Pair_button.timer_state==true&&!Timer_Timeout(&Pair_button.timePressed))
     {
        Pair_button.timer_state = false;
        if(UserData_getPointer()->SystemData.Provision_state == PROVISION_SUCCESS)
        {
            LOG("Pair button is not pressed, but the provision is success\r\n");
            Proc_Change_ProcCmd(PROC_LORA, CMD_LORA_SEND_DATA);
        }
     }
     else if(Pair_button.isPressed == false)
     {
        LOG("Pair button is not pressed\r\n");
        Pair_button.timer_state = false;
     }
#if LCD_CONTROLER
     for(uint8_t index=0; index<5; index++)
     {
         if(Button[index].isPressed == true)
         {
            LCD_Refresh_remainTime();
            Sensor_t *Sensor = Sensor_get_value();
            Button[index].isPressed = false;
            switch (index)
            {
				case GPIO_BUTTON_LEFT:
				case GPIO_BUTTON_RIGHT:
				case GPIO_BUTTON_UP:
				   Sensor->Relay[index] = (Sensor->Relay[index] == 0)?1:0;
				   Proc_Change_ProcCmd(PROC_RELAY, CMD_RELAY);
               Proc_relay_refreshTimer();
               Sensor->Relay_isChange = RELAY_CHANGE_BY_BUTTON;
				   break;
				case GPIO_BUTTON_BOTTOM:
				   Sensor->LCD_state = (Sensor->LCD_state == 0)?1:0;
				   LCD_power(Sensor->LCD_state);
				   if(Sensor->LCD_state)
				   {
					   LCD_initScreen();
					   Proc_DWIN(WRITE_RELAY_BUTTON, LCD_RELAY_1, (uint8_t*)&Sensor->Relay[0], 0,0x01);
				   }
				   break;
				default:
				   break;
            }
         }
     }
#endif
 }
