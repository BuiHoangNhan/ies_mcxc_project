#include "Proc_LCD.h"
#include <2_Middle/4_Sensor/Sensor.h>
#if LCD_CONTROLER
static void LCD_DWIN_Update_UIP(void)
{
    Sensor_t *sensor = Sensor_get_value();
    struct_LCD_Data str_LCD_Data;
    // str_LCD_Data.U1 = (uint16_t)sensor->BL0942_sensor_data[0].voltage;
    // str_LCD_Data.I1 = (uint16_t)sensor->BL0942_sensor_data[0].current;
    // str_LCD_Data.P1 = (uint32_t)sensor->BL0942_sensor_data[0].power;
    // str_LCD_Data.U2 = (uint16_t)sensor->BL0942_sensor_data[1].voltage;
    // str_LCD_Data.I2 = (uint16_t)sensor->BL0942_sensor_data[1].current;
    // str_LCD_Data.P2 = (uint32_t)sensor->BL0942_sensor_data[1].power;
    // str_LCD_Data.U3 = (uint16_t)sensor->BL0942_sensor_data[2].voltage;
    // str_LCD_Data.I3 = (uint16_t)sensor->BL0942_sensor_data[2].current;
    // str_LCD_Data.P3 = (uint32_t)sensor->BL0942_sensor_data[2].power;
    str_LCD_Data.U1 = 0x1234;
    str_LCD_Data.I1 = 0x1234;
    str_LCD_Data.P1 = 0x12345678;
    str_LCD_Data.U2 = 0x1234;
    str_LCD_Data.I2 = 0x1234;  
    str_LCD_Data.P2 = 0x12345678;
    str_LCD_Data.U3 = 0x1234;
    str_LCD_Data.I3 = 0x1234;
    str_LCD_Data.P3 = 0x12345678;
    Proc_DWIN(WRITE_LCD_MULTI_BYTES, LCD_VOLTAGE_PHASE1,(uint8_t*)&str_LCD_Data, 0,0x01);
    Proc_DWIN(WRITE_LCD_2BYTES, LCD_RSSI,(uint8_t*)&str_LCD_Data, -20,0x01);
    
}

static void LCD_DWIN_Update_SETUP(void)
{
    
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
