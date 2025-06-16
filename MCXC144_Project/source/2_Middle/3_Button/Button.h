/*
 * Sensor.h
 *
 *  Created on: 25 Nov 2024
 *      Author: HuyDQ
 */

 #ifndef __BUTTON_H
 #define __BOTTON_H_
 
 #include <stdbool.h>
 #include "stdint.h"
 #include <3_Application/Sys_Process.h>
typedef struct __str_Button
{
    bool isPressed;
    bool timer_state;   
    timer_t  timePressed;
} strt_Button ;

void Button_Handler(void);

void Button_setState(uint8_t button_state);

#endif
 
