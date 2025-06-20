#include "Sensor.h"
#include <math.h>

#define R0 10000.0       // Reference resistance in ohms
#define T0 298.15        // Reference temperature in Kelvin (25°C)
#define BETA 3950.0      // Beta constant (adjust if necessary)
#define V_REF 3.3        // Reference voltage
#define R_FIXED 2.15     // Fixed resistor value in kOhms
#define ADC_MAX 4095
#define V_REF 3.3f

#if LCD_CONTROLER
Sensor_t Sensor = {.BL0942_CurChannel = BL0942_CHANNEL3,
                   .Alert = ALERT_NONE,
                   .Relay = {0, 0, 0},
                   .LCD_state = 1};
#elif NODE_NEMA
Sensor_t Sensor = {.Alert = ALERT_NONE};
#endif
extern volatile uint32_t Cur_systic;

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Sensor_get_value
*Description  : Return the pointer of Sensor data.
*Input        : None.
*Output       : Sensor.
*-----------------------------------------------------------------------------*/

Sensor_t *Sensor_get_value(void)
{
    return &Sensor;
}

#if   LCD_CONTROLER 

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Sensor_update_NodeValue
*Description  : Update value of all sensor to struct msg data before update.
*Input        : Lora_LCDCtrl_data_t *Node_data.
*Output       : None.
*-----------------------------------------------------------------------------*/
void Sensor_update_NodeValue(Lora_LCDCtrl_data_t *Node_data)
{
    Node_data->U1 = (int16_t)(Sensor.BL0942_sensor_data[0].voltage*100);
    Node_data->I1 = (int16_t)(Sensor.BL0942_sensor_data[0].current*100);
    Node_data->P1 = (int32_t)(Sensor.BL0942_sensor_data[0].power*100);
    Node_data->U2 = (int16_t)(Sensor.BL0942_sensor_data[1].voltage*100);
    Node_data->I2 = (int16_t)(Sensor.BL0942_sensor_data[1].current*100);
    Node_data->P2 = (int32_t)(Sensor.BL0942_sensor_data[1].power*100);
    Node_data->U3 = (int16_t)(Sensor.BL0942_sensor_data[2].voltage*100);
    Node_data->I3 = (int16_t)(Sensor.BL0942_sensor_data[2].current*10);
    Node_data->P3 = (int32_t)(Sensor.BL0942_sensor_data[2].power*100);
    Node_data->relay1 = Sensor.Relay[0];
    Node_data->relay2 = Sensor.Relay[1];
    Node_data->relay3 = Sensor.Relay[2];
    Node_data->uptime = Cur_systic;
    Node_data->alert = Sensor.Alert;
}
#elif NODE_NEMA

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: Sensor_update_NodeValue
*Description  : Update value of all sensor to struct msg data before update.
*Input        : Lora_Nema_data_t *Node_data.
*Output       : None.
*-----------------------------------------------------------------------------*/
void Sensor_update_NodeValue(Lora_Nema_data_t *Node_data)
{
    Node_data->U = (uint16_t)Sensor.ATM90e26_sensor_data.voltage*100;
    Node_data->I = (uint16_t)Sensor.ATM90e26_sensor_data.current*100;
    Node_data->P = (uint16_t)Sensor.ATM90e26_sensor_data.power*100;
    Node_data->temperature = (uint16_t)Sensor.Temperature*100;
    Node_data->brightness = (uint32_t)Sensor.Brightness;
    Node_data->light = (uint16_t)Sensor.Dim_value;
    Node_data->uptime = Cur_systic;
    Node_data->alert = Sensor.Alert;
    // Log all values
    LOGF("Sensor Data Updated:\r\n");
    LOGF( "Voltage (U): %u (x0.01 V)\r\n", Node_data->U);
    LOGF("Current (I): %u (x0.01 A)\r\n", Node_data->I);
    LOGF("Power   (P): %u (x0.01 W)\r\n", Node_data->P);
    LOGF("Temperature : %u (x0.01 C)\r\n", Node_data->temperature);
    LOGF("Brightness  : %lu\r\n", Node_data->brightness);
    LOGF("Light level : %u\r\n", Node_data->light);
    LOGF("Uptime (ms) : %lu\r\n", Node_data->uptime);
    switch (Node_data->alert)
    {
	case ALERT_NONE:
		LOG("Alert: None\r\n");
		break;
	case ALERT_OVER_VOLTAGE:
		LOG("Alert: Over Voltage\r\n");
		break;
	case ALERT_OVER_CURRENT:
		LOG("Alert: Over Current\r\n");
		break;
	case ALERT_OVER_POWER:
		LOG("Alert: Over Power\r\n");
		break;
    }
}

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: NEMA_Get_temp
*Description  : Parse data of temperature sensor adc value.
*Input        : uint16_t adc_value.
*Output       : None.
*-----------------------------------------------------------------------------*/
void NEMA_Get_temp(uint16_t adc_value)
{

    float ntc_temp_mcu = (V_REF * adc_value) / ADC_MAX;

    // Calculate the resistance of the thermistor (R1)
    float R1 = (ntc_temp_mcu * R_FIXED) / (V_REF - ntc_temp_mcu);

    // Calculate temperature in Kelvin using the exponential equation
    float temp_kelvin = 1.0f / ((logf(R1 / R0) / BETA) + (1.0f / T0));

    // Convert Kelvin to Celsius
    Sensor.Temperature = (uint16_t)(temp_kelvin - 273.15f);
}

/*------------------------------------------------------------------------------
*Engineer     : HuyDoan
*Historical   : 1. November 19, 2024
*Function name: NEMA_Get_brightness
*Description  : Update value of all sensor to struct msg data before update.
*Input        : uint16_t brightness.
*Output       : None.
*-----------------------------------------------------------------------------*/
void NEMA_Get_brightness(uint16_t brightness)
{
    Sensor.Brightness = brightness;
}
#endif
