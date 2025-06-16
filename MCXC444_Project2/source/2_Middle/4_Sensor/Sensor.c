#include "Sensor.h"
#include <math.h>

#define R0 10000.0       // Reference resistance in ohms
#define T0 298.15        // Reference temperature in Kelvin (25°C)
#define BETA 3950.0      // Beta constant (adjust if necessary)
#define V_REF 3.3        // Reference voltage
#define R_FIXED 2.15     // Fixed resistor value in kOhms
#define ADC_MAX 4095
#define V_REF 3.3f

Sensor_t Sensor;
extern volatile uint32_t Cur_systic;

void Sensor_update_value(void)
{
#if   LCD_CONTROLER 
    for(enum_BL0942_Channel i = 0; i<NUM_POWER_SENSOR_CHANNEL;i++)
    {
        Switch_channel(i);
        BL0942_request();
        BL0942_process(&Sensor.BL0942_sensor_data[i]);
    }
#elif NODE_NEMA    
    Sensor.Brightness = 0;
    Sensor.Temperature = 0;
    BL0942_request();
    BL0942_process(&Sensor.BL0942_sensor_data);
#endif
}

Sensor_t *Sensor_get_value(void)
{
    return &Sensor;
}
#if   LCD_CONTROLER 
void Sensor_update_NodeValue(Lora_LCDCtrl_data_t *Node_data)
{
    Node_data->U1 = (uint16_t)Sensor.BL0942_sensor_data[0].voltage;
    Node_data->I1 = (uint16_t)Sensor.BL0942_sensor_data[0].current;
    Node_data->P1 = (uint32_t)Sensor.BL0942_sensor_data[0].power;
    Node_data->U2 = (uint16_t)Sensor.BL0942_sensor_data[1].voltage;
    Node_data->I2 = (uint16_t)Sensor.BL0942_sensor_data[1].current;
    Node_data->P2 = (uint32_t)Sensor.BL0942_sensor_data[1].power;
    Node_data->U3 = (uint16_t)Sensor.BL0942_sensor_data[2].voltage;
    Node_data->I3 = (uint16_t)Sensor.BL0942_sensor_data[2].current;
    Node_data->P3 = (uint32_t)Sensor.BL0942_sensor_data[2].power;
    Node_data->relay1 = Sensor.Relay[0];
    Node_data->relay2 = Sensor.Relay[1];
    Node_data->relay3 = Sensor.Relay[2];
    Node_data->uptime = Cur_systic;
}
#elif NODE_NEMA
void Sensor_update_NodeValue(Lora_Nema_data_t *Node_data)
{
    Node_data->U = (uint16_t)Sensor.BL0942_sensor_data.voltage;
    Node_data->I = (uint16_t)Sensor.BL0942_sensor_data.current;
    Node_data->P = (uint16_t)Sensor.BL0942_sensor_data.power;
    Node_data->temperature = (uint16_t)Sensor.Temperature;
    Node_data->brightness = (uint32_t)Sensor.Brightness;
    Node_data->light = (uint16_t)Sensor.Dim_value;
    Node_data->uptime = Cur_systic;
}
void NEMA_Get_temp(uint16_t *temp)
{

    float ntc_temp_mcu = (V_REF * adc_value) / ADC_MAX;

    // Calculate the resistance of the thermistor (R1)
    float R1 = (ntc_temp_mcu * R_FIXED) / (V_REF - ntc_temp_mcu);

    // Calculate temperature in Kelvin using the exponential equation
    float temp_kelvin = 1.0f / ((logf(R1 / R0) / BETA) + (1.0f / T0));

    // Convert Kelvin to Celsius
    return temp_kelvin - 273.15f;
}
void NEMA_Get_brightness(uint16_t *brightness)
{
    *brightness = Sensor.Brightness;
}
#endif
