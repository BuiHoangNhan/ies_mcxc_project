#include "Proc_Led_Button_Relay_Dimmer.h"
#include <3_Application/Sys_Process.h>
#include <1_Hardware/3_Led_IO/IO_driver.h>
#include <2_Middle/4_Sensor/Sensor.h>
static  uint16_t CurDim_value = 0;
#if LCD_CONTROLER
static timer_t relay_timer;

static void Proc_led(void)
{
    Sensor_t *Sensor = Sensor_get_value();
    Led_set(GPIO_LED_1, (Sensor->Led[0]!=0));
    Led_set(GPIO_LED_2, (Sensor->Led[1]!=0));
    Led_set(GPIO_LED_3, (Sensor->Led[2]!=0));
}
static void Proc_relay(void)
{
    Sensor_t *Sensor = Sensor_get_value();
    Proc_DWIN(WRITE_RELAY_BUTTON, LCD_RELAY_1, (uint8_t*)&Sensor->Relay[0], 0,0x01);
    Relay_set(GPIO_GPIO_RELAY_1, (Sensor->Relay[0]!=0));
    Relay_set(GPIO_GPIO_RELAY_2, (Sensor->Relay[1]!=0));
    Relay_set(GPIO_GPIO_RELAY_3, (Sensor->Relay[2]!=0));
}
void Proc_relay_refreshTimer(void)
{
    Timer_Create(&relay_timer, RELAY_CHANGE_BY_LCD_BUTTON_TIMEOUT);
}
void Proc_relay_Timeout(void)
{
    Sensor_t *Sensor = Sensor_get_value();
    if((Sensor->Relay_isChange == RELAY_CHANGE_BY_LCD||
            Sensor->Relay_isChange == RELAY_CHANGE_BY_BUTTON)
                &&Timer_Timeout(&relay_timer))
    {
        Sensor->Relay_isChange = RELAY_CHANGE_NONE;
        if(UserData_getPointer()->SystemData.Provision_state == PROVISION_SUCCESS)
        {
            Proc_Change_ProcCmd(PROC_LORA, CMD_LORA_SEND_DATA);
        }
    }
}
#elif NODE_NEMA
void Proc_dimmer(void)
{
	LOG("Nodes start dimming the LED\r\n");
    Sensor_t *Sensor = Sensor_get_value();
    DimOff_set((Sensor->Dim_value!=0));
    bool DimStatus = false;
    uint16_t DimThreshold = Sensor->Dim_value; 
    while(!DimStatus)
    {
        if(CurDim_value == DimThreshold)
        {
            DimStatus = true;
        }
        else if(CurDim_value < DimThreshold)
        {
            CurDim_value++;
            Dimmer_set((uint16_t)(CurDim_value*V_LED_DIMMER_SCALE));
        }
        else
        {
            CurDim_value--;
            Dimmer_set((uint16_t)(CurDim_value*V_LED_DIMMER_SCALE));
        }
        System_DelayMs(20);
    }
    CurDim_value = DimThreshold;
    LOG("Nodes finish dimming the LED\r\n");
}
#endif
void Proc_Led_Button_Relay_Dimmer(void)
{
    switch (Proc_GetCmd())
    {
#if LCD_CONTROLER
        case CMD_RELAY:
            Proc_relay();
            break;
#elif NODE_NEMA
        case CMD_DIMMER:
            Proc_dimmer();
            break;
#endif
    }
}

