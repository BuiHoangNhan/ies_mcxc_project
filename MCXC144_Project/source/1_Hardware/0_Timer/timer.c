/*
 * timer.c
 *
 *  Created on: 22 Nov 2024
 *      Author: ADMIN
 */


#include "timer.h"
#include "fsl_clock.h"
#include "fsl_lptmr.h"  
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TIMER_LPTMR_BASE   LPTMR0
#define TIMER_LPTMR_IRQn   LPTMR0_IRQn
#define LPTMR_LED_HANDLER LPTMR0_IRQHandler
/* Get source clock for LPTMR driver */
#define LPTMR_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_LpoClk)
/* Define LPTMR microseconds counts value */
#define LPTMR_USEC_COUNT 1000000U
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool utickExpired;
static volatile uint64_t count;
static volatile uint32_t systemDelayTicks;
lptmr_config_t lptmrConfig;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
void LPTMR_LED_HANDLER(void)
{
  LPTMR_ClearStatusFlags(TIMER_LPTMR_BASE, kLPTMR_TimerCompareFlag);
    count ++;
    if (systemDelayTicks != 0)
    {
        systemDelayTicks --;
    }
    __DSB();
    __ISB();
}


void Timer_Init(void)
{
  LPTMR_GetDefaultConfig(&lptmrConfig);

  /* Initialize the LPTMR */
  LPTMR_Init(TIMER_LPTMR_BASE, &lptmrConfig);
  LPTMR_SetTimerPeriod(TIMER_LPTMR_BASE, USEC_TO_COUNT(LPTMR_USEC_COUNT, LPTMR_SOURCE_CLOCK)/1000);

  /* Enable timer interrupt */
  LPTMR_EnableInterrupts(TIMER_LPTMR_BASE, kLPTMR_TimerInterruptEnable);

  /* Enable at the NVIC */
  EnableIRQ(TIMER_LPTMR_IRQn);
  LPTMR_StartTimer(TIMER_LPTMR_BASE);
}

void SysTick_Handler(void)
{

}

void System_DelayMs(uint32_t ms)
{
  systemDelayTicks = ms;
  while (systemDelayTicks)
	  {};
}

void Timer_Create(timer_t* t, uint64_t intervalMs)
{
  t->totalTickMs = count + intervalMs;
}

uint64_t Timer_Timeout(timer_t* t)
{
  return ((count > t->totalTickMs) ? 1 : 0);
}
