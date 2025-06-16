#ifndef __SENSOR_DRIVER_H_ 
#define __SENSOR_DRIVER_H_

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_adc16.h"
#include <stdint.h>
#include <stdbool.h>
        /*******************************************************************************
                                * Definitions
         ******************************************************************************/
#define SENSOR_ADC16_BASE          ADC0
#define SENSOR_ADC16_CHANNEL_GROUP 0U

#define SENSOR_ADC16_IRQn             ADC0_IRQn
#define SENSOR_ADC16_IRQ_HANDLER_FUNC ADC0_IRQHandler

typedef enum __enumSensor_adc_channel {
    SENSOR_ADC16_LIGHT_CHANNEL = 8,
    SENSOR_ADC16_TEMPERATURE_CHANNEL = 9
} enumSensor_adc_channel;
typedef struct __strtSensor_adc {
    volatile uint32_t AdcValue;
    volatile bool AdcStatus;
} strtSensor_adc;


uint32_t Sensor_ADC_request(enumSensor_adc_channel channel); 

void Sensor_ADC_init(void);
#endif