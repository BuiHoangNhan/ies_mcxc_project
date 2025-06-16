/*
 * Sensor.h
 *
 *  Created on: 25 Nov 2024
 *      Author: HuyDQ
 */

#ifndef __CONTROL_H_
#define __CONTROL_H_

#include <stdbool.h>
#include "stdint.h"
#include <2_Middle/2_LCD/LCD_Dwin.h>
#include <2_Middle/5_BL0942/BL0942.h>
#include <2_Middle/1_Lora/lora_msg.h>   
#include <1_Hardware/6_Sensor/Sensor_driver.h>
#if   LCD_CONTROLER 
#define NUM_RELAY_CHANNEL 3
#define NUM_LED 3
#define NUM_POWER_SENSOR_CHANNEL 3
#endif

typedef enum __enum_AlertType
{
    ALERT_NONE = 0,
    ALERT_OVER_VOLTAGE = 1<<0,
    ALERT_OVER_CURRENT = 1<<1,
    ALERT_OVER_POWER = 1<<2,
#if NODE_NEMA
    ALERT_OVER_HEAT = 1<<3,
#endif
} enum_AlertType;

#if LCD_CONTROLER
typedef enum __enum_relayType_change
{
    RELAY_CHANGE_NONE = 0,
    RELAY_CHANGE_BY_BUTTON = 1,
    RELAY_CHANGE_BY_LCD = 2,
    RELAY_CHANGE_BY_MSG = 3,
} enum_relayType_change;
#endif

#pragma pack(1)
typedef struct {
#if   LCD_CONTROLER 
    BL0942_Data BL0942_sensor_data[NUM_POWER_SENSOR_CHANNEL];
    enum_BL0942_Channel BL0942_CurChannel;
    uint8_t Relay[NUM_RELAY_CHANNEL];
    enum_relayType_change Relay_isChange;
    uint8_t Led[NUM_LED];
    bool LCD_state;
#elif NODE_NEMA
    BL0942_Data ATM90e26_sensor_data;
    uint16_t Brightness;
    uint16_t Temperature;
    uint16_t Dim_value;
#endif
    enum_AlertType Alert;
} Sensor_t;

#pragma pack()


Sensor_t *Sensor_get_value(void);
void NEMA_Get_temp(uint16_t adc_value);
void NEMA_Get_brightness(uint16_t brightness);
#endif
