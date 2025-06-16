#include "Proc_LCD.h"
#include <2_Middle/4_Sensor/Sensor.h>
#if LCD_CONTROLER
extern int8_t RSSI ;
static void LCD_DWIN_Update_UIP(void)
{
    Sensor_t *sensor = Sensor_get_value();
    struct_LCD_Data str_LCD_Data;
    str_LCD_Data.U1 = SWAP16((int16_t)(sensor->BL0942_sensor_data[0].voltage*100));
    str_LCD_Data.I1 = SWAP16((int16_t)(sensor->BL0942_sensor_data[0].current*100));
    str_LCD_Data.P1 = SWAP32((int32_t)(sensor->BL0942_sensor_data[0].power*100));
    str_LCD_Data.U2 = SWAP16((int16_t)(sensor->BL0942_sensor_data[1].voltage*100));
    str_LCD_Data.I2 = SWAP16((int16_t)(sensor->BL0942_sensor_data[1].current*100));
    str_LCD_Data.P2 = SWAP32((int32_t)(sensor->BL0942_sensor_data[1].power*100));
    str_LCD_Data.U3 = SWAP16((int16_t)(sensor->BL0942_sensor_data[2].voltage*100));
    str_LCD_Data.I3 = SWAP16((int16_t)(sensor->BL0942_sensor_data[2].current*100));
    str_LCD_Data.P3 = SWAP32((int32_t)(sensor->BL0942_sensor_data[2].power*100));
    Proc_DWIN(WRITE_LCD_MULTI_BYTES, LCD_VOLTAGE_PHASE1,(int8_t*)&str_LCD_Data, 0,0x01);
    Proc_DWIN(WRITE_LCD_2BYTES, LCD_RSSI,(uint8_t*)&str_LCD_Data, RSSI,0x01);
    
}

static void LCD_DWIN_Update_SETUP(void)
{
      WRITE_LCD_INIT_INFO:
    
}

static void LCD_DWIN_Update_UIPTH(void)
{

}
void Proc_LCD(void)
{
    switch (Proc_GetCmd())
    {
    case CMD_LCD_UPDATE_UIP:
        LCD_DWIN_Update_UIP();
        break;
    case CMD_LCD_UPDATE_SETUP:
        LCD_DWIN_Update_SETUP();
        break;
    case CMD_LCD_UPDATE_UIPTH:
        LCD_DWIN_Update_UIPTH();
        break;
    default:
        break;
    }
}
#endif
