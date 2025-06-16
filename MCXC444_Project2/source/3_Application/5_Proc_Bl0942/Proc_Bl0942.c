

#include <stdbool.h>
#include "Proc_Bl0942.h"
static uint8_t Bl0942_buf[30];
void Bl0942_init(void)
{
#if   LCD_CONTROLER    
    Ac_sensing_sel_init();
#endif
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
    //TODO: 00 01 10 to CHANNEL 1 ,2 3
}
#endif

void Proc_bl0942(void)
{
    Sensor_t *Sensor = Sensor_get_value();
    switch(Proc_GetCmd())
    {
        case CMD_BL0942:
#if  LCD_CONTROLER        
            for(enum_BL0942_Channel channel = 0; channel<NUM_POWER_SENSOR_CHANNEL;channel++)
            {
                Switch_channel(channel);
                BL0942_requestFull(Bl0942_buf);
                BL0942_processFull(Bl0942_buf, BL0942_FULL_PACKET_RECEIVE_LEN, &Sensor->BL0942_sensor_data[channel]);
            }
#elif   NODE_NEMA
            BL0942_requestFull(Bl0942_buf);
            BL0942_processFull(Bl0942_buf, BL0942_FULL_PACKET_RECEIVE_LEN, &Sensor->BL0942_sensor_data);
#endif
            break;
        default:
            break;
    }
}
