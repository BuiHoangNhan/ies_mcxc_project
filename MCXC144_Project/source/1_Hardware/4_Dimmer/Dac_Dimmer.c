#include "Dac_Dimmer.h"
#if NODE_NEMA
void Dac_Dimmer_init(void)
{
    dac_config_t dacConfigStruct;
    DAC_GetDefaultConfig(&dacConfigStruct);
    DAC_Init(DAC_DIMMER_BASE, &dacConfigStruct);
    DAC_Enable(DAC_DIMMER_BASE, true);  
    DAC_SetBufferReadPointer(DAC_DIMMER_BASE, 0U);  
    DAC_SetBufferValue(DAC_DIMMER_BASE, 0U, 0 );    
}

void Dimmer_set(uint16_t Dim_value)
{
    DAC_SetBufferValue(DAC_DIMMER_BASE, 0U, (uint16_t)Dim_value );
}
#endif
/* end of file  */