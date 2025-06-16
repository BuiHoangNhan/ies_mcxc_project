#ifndef __PROC_LED_BUTTON_RELAY_H
#define __PROC_LED_BUTTON_RELAY_H

#include <stdlib.h>
#include <stdint.h>
#include <1_Hardware/4_Dimmer/Dac_Dimmer.h>

#define V_RANGE_DAC_REF                       (100)

#define V_DAC_CONVERT                	    (1.0/4095) /* 12bit DAC*/
#define V_LED_DIMMER_SCALE               	(1/(V_RANGE_DAC_REF*V_DAC_CONVERT))
void Proc_Led_Button_Relay_Dimmer(void);

#if LCD_CONTROLER
#define RELAY_CHANGE_BY_LCD_BUTTON_TIMEOUT 2*1000 // 60s
void Proc_relay_refreshTimer(void);
void Proc_relay_Timeout(void);
void Proc_dimmer(void);
#endif

#endif //__PROC_LED_BUTTON_RELAY_H
