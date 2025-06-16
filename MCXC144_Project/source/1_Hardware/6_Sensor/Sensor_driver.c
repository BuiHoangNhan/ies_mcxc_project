/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if NODE_NEMA
#include "Sensor_driver.h" 
#include <2_Middle/4_Sensor/Sensor.h>


static adc16_channel_config_t strAdc16_temp_cfg = {
    .channelNumber = SENSOR_ADC16_TEMPERATURE_CHANNEL,
    .enableInterruptOnConversionCompleted = true,
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    .enableDifferentialConversion = false,
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */
};

static adc16_channel_config_t strAdc16_light_cfg = {
    .channelNumber = SENSOR_ADC16_LIGHT_CHANNEL,
    .enableInterruptOnConversionCompleted = true,
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    .enableDifferentialConversion = false,
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */
};

static volatile strtSensor_adc strSensor_temp;
static volatile strtSensor_adc strSensor_light;
  /*******************************************************************************
  * Code
  ******************************************************************************/
 
 
void SENSOR_ADC16_IRQ_HANDLER_FUNC(void)
{
    if(strSensor_temp.AdcStatus == false)
    {
        strSensor_temp.AdcValue = ADC16_GetChannelConversionValue(SENSOR_ADC16_BASE, SENSOR_ADC16_CHANNEL_GROUP);
        NEMA_Get_temp(strSensor_temp.AdcValue);
        strSensor_temp.AdcStatus = true;
    }
    if(strSensor_light.AdcStatus == false)
    {
        strSensor_light.AdcValue = ADC16_GetChannelConversionValue(SENSOR_ADC16_BASE, SENSOR_ADC16_CHANNEL_GROUP);
        NEMA_Get_brightness(strSensor_light.AdcValue);
        strSensor_light.AdcStatus = true;
    }
    SDK_ISR_EXIT_BARRIER;
}
 
void Sensor_ADC_init(void)
{
    adc16_config_t strAdc16_Config;
 
    EnableIRQ(SENSOR_ADC16_IRQn);
 
     /*
      * adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceVref;
      * adc16ConfigStruct.clockSource = kADC16_ClockSourceAsynchronousClock;
      * adc16ConfigStruct.enableAsynchronousClock = false;
      * adc16ConfigStruct.clockDivider = kADC16_ClockDivider8;
      * adc16ConfigStruct.resolution = kADC16_ResolutionSE12Bit;
      * adc16ConfigStruct.longSampleMode = kADC16_LongSampleDisabled;
      * adc16ConfigStruct.enableHighSpeed = false;
      * adc16ConfigStruct.enableLowPower = false;
      * adc16ConfigStruct.enableContinuousConversion = false;
      */
    ADC16_GetDefaultConfig(&strAdc16_Config);
 #ifdef BOARD_ADC_USE_ALT_VREF
 strAdc16_Config.referenceVoltageSource = kADC16_ReferenceVoltageSourceValt;
 #endif
    ADC16_Init(SENSOR_ADC16_BASE, &strAdc16_Config);
    ADC16_SetChannelMuxMode(SENSOR_ADC16_BASE , kADC16_ChannelMuxB);
    ADC16_EnableHardwareTrigger(SENSOR_ADC16_BASE, false); /* Make sure the software trigger is used. */
 #if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    ADC16_DoAutoCalibration(SENSOR_ADC16_BASE);
 #endif /* FSL_FEATURE_ADC16_HAS_CALIBRATION */
    strSensor_temp.AdcStatus = true;
    strSensor_light.AdcStatus = true;
 }
 void Sensor_ADC_request(enumSensor_adc_channel channel)
 {
    switch (channel)
    {
        case SENSOR_ADC16_LIGHT_CHANNEL:
            ADC16_SetChannelConfig(SENSOR_ADC16_BASE, SENSOR_ADC16_CHANNEL_GROUP, &strAdc16_light_cfg);
            strSensor_light.AdcStatus = false;
            break;
        case SENSOR_ADC16_TEMPERATURE_CHANNEL:
            strSensor_temp.AdcStatus = false;
            ADC16_SetChannelConfig(SENSOR_ADC16_BASE, SENSOR_ADC16_CHANNEL_GROUP, &strAdc16_temp_cfg);
            break;
        default:    
            break;
    }
 }
 strtSensor_adc* p_Sensor_ADC_get(enumSensor_adc_channel channel)
{
	switch (channel)
	{
		case SENSOR_ADC16_LIGHT_CHANNEL:
			return &strSensor_light;
			break;
		case SENSOR_ADC16_TEMPERATURE_CHANNEL:
			return &strSensor_temp;
			break;
		default:
			return 0;
			break;
	}
}
#endif
