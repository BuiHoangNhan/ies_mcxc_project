#ifndef __PROC_BL0942_H
#define __PROC_BL0942_H
#include <1_Hardware/3_Led_IO/IO_driver.h>
#include <1_Hardware/5_Uart_BL0942/Uart_BL0942.h>
#include <2_Middle/5_BL0942/BL0942.h>
#include <2_Middle/4_Sensor/Sensor.h>
#include <3_Application/Sys_Process.h>

void Bl0942_init(void);

void Proc_bl0942(void);
#endif 
/* end of __PROC_BL0942_H */