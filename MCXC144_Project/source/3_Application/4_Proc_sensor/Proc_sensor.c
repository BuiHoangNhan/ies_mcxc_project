#include <3_Application/4_Proc_sensor/Proc_sensor.h>
#include <3_Application/Sys_Process.h>

void Proc_sensor(void)
{
    Sensor_t *Sensor = Sensor_get_value();
    switch (Proc_GetCmd())
    {
#if NODE_NEMA
        case CMD_SENSOR_LIGHT:
        	LOG("Node is taking data from Sensor Light\r\n");
            Sensor_ADC_request(SENSOR_ADC16_LIGHT_CHANNEL);
        break;
        case CMD_SENSOR_TEMP:
        	LOG("Node is taking data from Sensor Temperature\r\n");
            Sensor_ADC_request(SENSOR_ADC16_TEMPERATURE_CHANNEL);    
        break;
        case CMD_ATM90E26:
        	LOG("Node is taking data from ATM90E26\r\n");
        	float Voltage_temp = GetLineVoltage();
            if(Voltage_temp!=0xffff&&Voltage_temp>0)
            {
            	Sensor->ATM90e26_sensor_data.voltage = Voltage_temp;
            }
			float Current_temp = GetLineCurrent();
            if(Current_temp!=0xffff)
            {
            	Current_temp = (float)Current_temp*2.0939-0.0398;
            	if(Current_temp>0&&Current_temp<1.5)
            	{
            		Sensor->ATM90e26_sensor_data.current = (float)Current_temp*2.0939-0.0398;
            	}
            	else if(Current_temp<0)
				{
            		Sensor->ATM90e26_sensor_data.current  = 0;
				}
            }
            Sensor->ATM90e26_sensor_data.power = Sensor->ATM90e26_sensor_data.voltage*Sensor->ATM90e26_sensor_data.current;

            GetFrequency();
            break;
#endif
    }
}

