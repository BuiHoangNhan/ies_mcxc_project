

#include <stdbool.h>
#include "Proc_Bl0942.h"
#if LCD_CONTROLER
extern volatile uint8_t Bl0942_buf[30];
extern volatile uint8_t index_bl0942 ;

void Bl0942_init(void)
{  
    Ac_sensing_sel_init();
    BL0942_UART_Init();
}
#if   LCD_CONTROLER
static void Switch_channel(enum_BL0942_Channel chanel)
{
    switch (chanel)
    {
    case BL0942_CHANNEL1:
        Ac_sensing_sel(GPIO_AC_SENSING_MUX_SELA_PIN,0);
        Ac_sensing_sel(GPIO_AC_SENSING_MUX_SELB_PIN,0);
        break;
    case BL0942_CHANNEL2:
        Ac_sensing_sel(GPIO_AC_SENSING_MUX_SELA_PIN,1);
        Ac_sensing_sel(GPIO_AC_SENSING_MUX_SELB_PIN,0);
        break;
    case BL0942_CHANNEL3:
        Ac_sensing_sel(GPIO_AC_SENSING_MUX_SELA_PIN,0);
        Ac_sensing_sel(GPIO_AC_SENSING_MUX_SELB_PIN,1);
        break;    
    default:
        break;
    };
    System_DelayMs(5);
}
#endif

void Proc_bl0942(void)
{
    switch(Proc_GetCmd())
    {
        case CMD_BL0942:
            Proc_bl0942_request_data();
            break;
        default:
            break;
    }
}
void Proc_bl0942_request_data(void)
{
    Sensor_t *Sensor = Sensor_get_value();
    if(Sensor->BL0942_CurChannel == NUM_POWER_SENSOR_CHANNEL-1)
    {
        Sensor->BL0942_CurChannel = 0;
    }
    else
    {
        Sensor->BL0942_CurChannel++;
    }
    Switch_channel(Sensor->BL0942_CurChannel);
    memset(Bl0942_buf,0,sizeof(Bl0942_buf));
    index_bl0942 =0;
    Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].status = BL0942_PROGRESS;
    BL0942_requestFull(Bl0942_buf);
}
void Proc_bl0942_process_data(void)
{
	Sensor_t *Sensor = Sensor_get_value();
    if(Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].status == BL0942_DONE)
    {
        Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].status = BL0942_NONE;
        BL0942_processFull(Bl0942_buf, BL0942_FULL_PACKET_RECEIVE_LEN, &Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel]);
    }  
}
#endif

