#include "Sys_Process.h"
#include <stdbool.h>
#include <stdint.h>

static structProcHandler ProcHandler[NUMBER_STACK_PROC] = {
    {PROC_NONE, CMD_NONE}, 
    {PROC_NONE, CMD_NONE}, 
    {PROC_NONE, CMD_NONE}, 
    {PROC_NONE, CMD_NONE}, 
    {PROC_NONE, CMD_NONE}
};
static structProcHandler curProcHandler = {PROC_NONE, CMD_NONE};
static uint8_t ProcHandlerHead, ProcHandlerTail;
structProcHandler   Proc_systick_proc;

void Proc_Change_ProcCmd(enum_ProcID Proc, enum_CMDID Cmd)
{
    ProcHandler[ProcHandlerHead].eProc = Proc;
    ProcHandler[ProcHandlerHead].eCmd = Cmd;
    if(ProcHandlerHead==NUMBER_STACK_PROC-1)
    {
        ProcHandlerHead = 0;
    }   
    else
    {
        ProcHandlerHead++;
    }
}

enum_ProcID Proc_GetProc(void)
{
    return ProcHandler[ProcHandlerTail].eProc;
}

enum_CMDID Proc_GetCmd(void)
{
    return ProcHandler[ProcHandlerTail].eCmd;
}
void Driver_Init(void)
{
    Systick_timer_init();
    HAL_FlashInit();
    UserData_init();
#if LCD_CONTROLER
    Button_init();
#endif
    Sensor_ADC_init();
    Bl0942_init();
    Lora_uart_init();
    Timer_Init();
    Lora_Init();
    // UserData_save();
#if LCD_CONTROLER
    LCD_DWIN_Init();
#elif NODE_NEMA
    DimOff_gpio_init();
    Dac_Dimmer_init();
#endif
}

static void Proc_Clear_ProcCmd(void)
{

    ProcHandler[ProcHandlerTail].eProc = PROC_NONE;
    ProcHandler[ProcHandlerTail].eCmd = CMD_NONE;
    if(ProcHandlerTail==NUMBER_STACK_PROC-1)
    {
        ProcHandlerTail = 0;
    }   
    else
    {
        ProcHandlerTail++;
    }
}
bool Proc_ProcCmdIsChanged(void)
{
    bool blIsChanged;
    if (curProcHandler.eCmd!=ProcHandler[1].eCmd || curProcHandler.eProc!=ProcHandler[1].eProc) 
    {
        blIsChanged = true;
    }
    else
    {
        blIsChanged = false;
    }
    return blIsChanged;
}
static void Proc_change_Systick_ProcCmd(void)
{
    if(Proc_systick_proc.eProc!=PROC_NONE)
    {
        Proc_Change_ProcCmd(Proc_systick_proc.eProc,Proc_systick_proc.eCmd);
        Proc_systick_proc.eProc = PROC_NONE;
        Proc_systick_proc.eCmd = CMD_NONE;
    }
}
static void Proc_add_Systick_Proc(enum_ProcID Proc, enum_CMDID Cmd)
{
    Proc_systick_proc.eProc = Proc;
    Proc_systick_proc.eCmd = Cmd;
}
void Proc_Process(void)
{
	Lora_MsgStatus_t *Provision_status = Lora_Provision_Succes();
     Proc_change_Systick_ProcCmd();
    if((*Provision_status==PROVISION_NONE))
    {
        Proc_Change_ProcCmd(PROC_LORA, CMD_LORA_PROVISION);
    }
	else if(*Provision_status!=PROVISION_NONE&&ProcHandler[ProcHandlerTail].eProc==PROC_LORA&&ProcHandler[ProcHandlerTail].eCmd == CMD_LORA_PROVISION)
	{
		Proc_Clear_ProcCmd();
	}
    curProcHandler = ProcHandler[ProcHandlerTail];
    switch (ProcHandler[ProcHandlerTail].eProc)
    {
    case PROC_BL0942:
        if(*Provision_status==PROVISION_SUCCESS)
        {
            Proc_bl0942();
        }        
        break;

#if LCD_CONTROLER
    case PROC_RELAY:
        if(*Provision_status==PROVISION_SUCCESS)
        {
            Proc_Led_Button_Relay_Dimmer();
        }
        break;
    case PROC_LCD_UPDATE:
        if(*Provision_status==PROVISION_SUCCESS)
        {
            Proc_LCD();
        }
        break;
#elif NODE_NEMA
    case PROC_DIMMER:
        if(*Provision_status==PROVISION_SUCCESS)
        {
            Proc_Led_Button_Relay_Dimmer();
        }
        break;
    case PROC_SENSOR:

        break;
#endif
    case PROC_LORA:
        Proc_Lora_Main();
        break;
    case PROC_NONE:
    default:
        break;

    }
    if(ProcHandler[ProcHandlerTail].eProc!=PROC_NONE)
    {
        Proc_Clear_ProcCmd();
    }

}

void Proc_System_ChangProc(void)
{
#if LCD_CONTROLER
    static uint16_t Tick_LCD = 0;
#elif NODE_NEMA
    static uint16_t Tick_Sensor = 0;
#endif
    static uint32_t Tick_Bl0942 = 0;
    static uint32_t Tick_Send_data = 0;
    Lora_MsgStatus_t *Provision_status = Lora_Provision_Succes();
    if(++Tick_Send_data >60000&&*Provision_status==PROVISION_SUCCESS&&(OTA_getState()==eOTA_Idle))
	{
		Tick_Send_data = 0;
		Proc_add_Systick_Proc(PROC_LORA, CMD_LORA_SEND_DATA);
	}
    else if(++Tick_Bl0942 >  30000&&*Provision_status==PROVISION_SUCCESS)
    {
        Proc_add_Systick_Proc(PROC_BL0942, CMD_BL0942);
        Tick_Bl0942 = 0;
    }
#if LCD_CONTROLER
    else if(++Tick_LCD > 2000&&*Provision_status==PROVISION_SUCCESS)
    {
        Tick_LCD = 0;
        Proc_add_Systick_Proc(PROC_LCD_UPDATE, CMD_LCD_UPDATE_UIP);
    }
#elif NODE_NEMA
    else if(++Tick_Sensor > 1000&&*Provision_status==)
    {
        Tick_Sensor = 0;
         Proc_Change_ProcCmd(PROC_SENSOR, CMD_SENSOR);
    }
#endif
}

