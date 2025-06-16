#ifndef __PROC_LED_BUTTON_RELAY_H
#define __PROC_LED_BUTTON_RELAY_H

#include <stdlib.h>
#include <stdint.h>
#include <1_Hardware/4_Dimmer/Dac_Dimmer.h>

#define V_RANGE_DAC_REF                       (100)

#define V_DAC_CONVERT                	    (1.0/4095) /* 12bit DAC*/
#define V_LED_DIMMER_SCALE               	(1/(V_RANGE_DAC_REF*V_DAC_CONVERT))
void Proc_Led_Button_Relay_Dimmer(void);

#endif //__PROC_LED_BUTTON_RELAY_H
