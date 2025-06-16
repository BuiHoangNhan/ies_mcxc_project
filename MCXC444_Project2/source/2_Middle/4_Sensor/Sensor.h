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
#endif
#pragma pack(1)
typedef struct {
#if   LCD_CONTROLER 
    BL0942_Data BL0942_sensor_data[NUM_POWER_SENSOR_CHANNEL];
    uint8_t BL0942_CurChannel;
    uint8_t Relay[NUM_RELAY_CHANNEL];
    uint8_t  Led[NUM_LED];
#elif NODE_NEMA
    BL0942_Data BL0942_sensor_data;   .
    uint16_t Brightness;
    uint16_t Temperature;
    uint16_t Dim_value;
#endif
} Sensor_t;

#pragma pack()

void Sensor_update_value(void);

Sensor_t *Sensor_get_value(void);
#endif
