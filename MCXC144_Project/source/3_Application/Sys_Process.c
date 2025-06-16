#include "Sys_Process.h"
#include <stdbool.h>
#include <stdint.h>

#define POWER_METER_CYCLE 1.5*1000 // 1.5s
#if   LCD_CONTROLER
#define LCD_CYCLE 2.5*1000 // 1s
#elif NODE_NEMA
#define SENSOR_DATA_CYCLE 2*1000 // 1s
#endif

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
    WDOG_timer_init();
    HAL_FlashInit();
    UserData_init();
    Timer_Init();
    Pair_gpio_init();
    Boot_gpio_init();
    Lora_Init();
#if LCD_CONTROLER
    Button_init();
    gpio_relay_init();
    gpio_led_init();
    LCD_DWIN_Init();
    Bl0942_init();
#elif NODE_NEMA
    Sensor_ADC_init();
    DimOff_gpio_init();
    Dac_Dimmer_init();
    ATM90E26_Init();
    Sensor_t *Sensor = Sensor_get_value();
    Sensor->Dim_value = 100; // Default value for dimmer
    Proc_dimmer();
#endif
    if(UserData_getPointer()->SystemData.Provision_state == PROVISION_SUCCESS)
    {
        Proc_Change_ProcCmd(PROC_LORA, CMD_LORA_SEND_DATA);
    }
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

static void Node_stateProc(void)
{
    Sensor_t *Sensor = Sensor_get_value();
    enum_AlertType CurALERT = ALERT_GET;
    enum_Provision_state *Provision_status = Lora_Provision_Succes();
    if(*Provision_status == PROVISION_SUCCESS)
    {   
        OTA_Timeout();
    }
#if   LCD_CONTROLER
    Proc_bl0942_process_data();
    Proc_relay_Timeout();
#endif
    if(CurALERT != Sensor->Alert &&*Provision_status==PROVISION_SUCCESS && OTA_getState() == eOTA_Idle)
    {
        // Proc_Change_ProcCmd(PROC_LORA, CMD_LORA_SEND_DATA);
    }
    Sensor->Alert = CurALERT;
}
void Proc_Process(void)
{
	enum_Provision_state *Provision_status = Lora_Provision_Succes();
    bool is_provisioned = (*Provision_status == PROVISION_SUCCESS);
    Proc_change_Systick_ProcCmd();
    if (*Provision_status == PROVISION_NONE) {
        Proc_Change_ProcCmd(PROC_LORA, CMD_LORA_PROVISION);
    } else if (ProcHandler[ProcHandlerTail].eProc == PROC_LORA && 
                ProcHandler[ProcHandlerTail].eCmd == CMD_LORA_PROVISION) {
        Proc_Clear_ProcCmd();
    }
    curProcHandler = ProcHandler[ProcHandlerTail];
    if (is_provisioned || ProcHandler[ProcHandlerTail].eProc == PROC_LORA) {
        switch (ProcHandler[ProcHandlerTail].eProc) {
#if LCD_CONTROLER
            case PROC_BL0942:
                Proc_bl0942();
                break;
                
            case PROC_RELAY:
                Proc_Led_Button_Relay_Dimmer();
                break;
                
            case PROC_LCD_UPDATE:
                Proc_LCD();
                break;
#elif NODE_NEMA
            case PROC_DIMMER:
                Proc_Led_Button_Relay_Dimmer();
                break;
                
            case PROC_SENSOR:
                Proc_sensor();
                break;
#endif
            case PROC_LORA:
                Proc_Lora_Main();
                break;
                
            case PROC_NONE:
            default:
                break;
        }
    }
    if(ProcHandler[ProcHandlerTail].eProc!=PROC_NONE) {
        Proc_Clear_ProcCmd();
    }

    if(*Provision_status == PROVISION_PENDING && OTA_getState() == eOTA_Idle)
    {
        if(Provision_pendingTimeout())
        {
            *Provision_status = PROVISION_NONE;
        }
    }
    if(is_provisioned) {
        Node_stateProc();
    }
#if LCD_CONTROLER
    LCD_RemainTime_check();
#endif
}

void Proc_System_ChangProc(void)
{
#if LCD_CONTROLER
    static uint16_t Tick_LCD = 0;
#elif NODE_NEMA
    static uint16_t Tick_Sensor_temp = 0;
    static uint16_t Tick_Sensor_light = 0;
#endif
    static uint16_t Tick_powerMeter = 0;

    static uint32_t Tick_Send_data = 0;
    enum_Provision_state *Provision_status = Lora_Provision_Succes();

    if(OTA_getState()==eOTA_Idle&&*Provision_status==PROVISION_SUCCESS)
    {
        if(++Tick_Send_data >60*1000)
        {
            Tick_Send_data = 0;
            Proc_add_Systick_Proc(PROC_LORA, CMD_LORA_SEND_DATA);
        }        
    else if(++Tick_powerMeter >  POWER_METER_CYCLE)
        {
            Tick_powerMeter = 0;
#if LCD_CONTROLER
            Proc_add_Systick_Proc(PROC_BL0942, CMD_BL0942);
#elif NODE_NEMA
            Proc_add_Systick_Proc(PROC_SENSOR, CMD_ATM90E26);
#endif
        }   
#if LCD_CONTROLER
        else if(++Tick_LCD > LCD_CYCLE)
        {
            Tick_LCD = 0;
            Proc_add_Systick_Proc(PROC_LCD_UPDATE, CMD_LCD_UPDATE_UIP);
        }
#elif NODE_NEMA
        else if(++Tick_Sensor_light > SENSOR_DATA_CYCLE)
        {
            Tick_Sensor_light = 0;
            Proc_add_Systick_Proc(PROC_SENSOR, CMD_SENSOR_LIGHT);
        }
        else if(++Tick_Sensor_temp > SENSOR_DATA_CYCLE)
        {
            Tick_Sensor_temp = 0;
            Proc_add_Systick_Proc(PROC_SENSOR, CMD_SENSOR_TEMP);
        }
#endif
    }
}

