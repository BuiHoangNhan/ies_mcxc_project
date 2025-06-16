#ifndef __DAC_DIMMER_H
#define __DAC_DIMMER_H
#if NODE_NEMA
#include <stdint.h>
#include "pin_mux.h"
#include "board.h"
#include "fsl_dac.h"

#define DAC_DIMMER_BASE DAC0

void Dac_Dimmer_init(void);
void Dimmer_set(uint16_t output);
#endif
#endif //__DAC_DIMMER_H