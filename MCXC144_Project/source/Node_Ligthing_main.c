/*
 * Copyright 2016-2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    MCXC144_Project.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include <3_Application/Sys_Process.h>
const char FW_VERSION[] = "1.0.7";
#if HW_PRETEST_FW
extern void HW_Test(void);
#endif
/* TODO: insert other include files here. */
#include "SEGGER_RTT.h"
/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif
    Driver_Init();
    uint8_t blIsBoot = Boot_button_read();
    enum_Provision_state isProvision = (UserData_getPointer()->SystemData.Provision_state == PROVISION_SUCCESS);

#if HW_PRETEST_FW
    if((blIsBoot==0)&&!isProvision)
    {
    	while(Pair_button_read()==1)
    	{}
    	HW_Test();
    }
#endif
	SEGGER_RTT_WriteString(0, "enter superloop\n");
    while(1){
    	SEGGER_RTT_WriteString(0, "running superloop\n");
        Button_Handler();
        Lora_Main();
        Proc_Process();
    }

    return 0 ;
}
