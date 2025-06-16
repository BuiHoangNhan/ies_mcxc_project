/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_pit.h"

#include "fsl_cop.h"
#include "fsl_rcm.h"
//#include <3_Application/Sys_Process.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SYSTICK_PIT_BASEADDR PIT
#define SYSTICK_PIT_CHANNEL  kPIT_Chnl_0
#define SYSTICK_TIMER_HANDLER   PIT_IRQHandler
#define PIT_IRQ_ID        PIT_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

/* Structure of initialize PIT */
pit_config_t pitConfig;
volatile uint32_t Cur_systic = 0;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/
void SYSTICK_TIMER_HANDLER(void)
{
    /* Clear interrupt flag.*/
    static uint16_t i = 0;
    if(i++>1000)
    {
        Cur_systic++;
        i =0 ;
        COP_Refresh(SIM);
    }

    Proc_System_ChangProc();
    PIT_ClearStatusFlags(SYSTICK_PIT_BASEADDR, SYSTICK_PIT_CHANNEL, kPIT_TimerFlag);
    /* Added for, and affects, all PIT handlers. For CPU clock which is much larger than the IP bus clock,
     * CPU can run out of the interrupt handler before the interrupt flag being cleared, resulting in the
     * CPU's entering the handler again and again. Adding DSB can prevent the issue from happening.
     */
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
void Systick_timer_init(void)
{
    /*
     * pitConfig.enableRunInDebug = false;
     */
    PIT_GetDefaultConfig(&pitConfig);

    /* Init pit module */
    PIT_Init(SYSTICK_PIT_BASEADDR, &pitConfig);

    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(SYSTICK_PIT_BASEADDR, SYSTICK_PIT_CHANNEL, USEC_TO_COUNT(1000U, PIT_SOURCE_CLOCK));

    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(SYSTICK_PIT_BASEADDR, SYSTICK_PIT_CHANNEL, kPIT_TimerInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(PIT_IRQ_ID);

    PIT_StartTimer(SYSTICK_PIT_BASEADDR, SYSTICK_PIT_CHANNEL);

}

void WDOG_timer_init(void)
{
    cop_config_t configCop;


    /*
     * configCop.enableWindowMode = false;
     * configCop.timeoutMode = kCOP_LongTimeoutMode;
     * configCop.enableStop = false;
     * configCop.enableDebug = false;
     * configCop.clockSource = kCOP_LpoClock;
     * configCop.timeoutCycles = kCOP_2Power10CyclesOr2Power18Cycles;
     */
    COP_GetDefaultConfig(&configCop);
#if FSL_FEATURE_COP_HAS_LONGTIME_MODE
    configCop.timeoutMode = kCOP_ShortTimeoutMode;
#endif
     COP_Init(SIM, &configCop);
    /* Check if WDOG reset occurred */
    if (RCM_GetPreviousResetSources(RCM) & kRCM_SourceWdog)
    {
        COP_Disable(SIM);
    }
    // else
    // {
    //     /* If WDOG reset is not occurred, enables COP */
    //     PRINTF("\r\nCOP example start!\r\n");
	COP_Init(SIM, &configCop);
    //     /* Refresh COP 10 times and then wait for timeout reset */
    //     for (uint32_t loopCount = 0U; loopCount < 10U; loopCount++)
    //     {
    //
    //         PRINTF("COP refresh %d time\r\n", loopCount + 1U);
    //     }

    //     PRINTF("COP will timeout and chip will be reset\r\n");
    // }
    // while (1)
    // {
    // }
}
