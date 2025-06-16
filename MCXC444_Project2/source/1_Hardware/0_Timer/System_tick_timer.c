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
#include <3_Application/Sys_Process.h>
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
	Cur_systic++;
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
