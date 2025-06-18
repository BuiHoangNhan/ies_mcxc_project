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
	LOG("Enter the main loop\r\n");
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif
    // Initialized Drivers
    Driver_Init();
    /* Storing the state of boot button
    *  1 = Button is pressed.
    *  0 = Button is not pressed.
    */
    uint8_t blIsBoot = Boot_button_read();
    enum_Provision_state isProvision = (UserData_getPointer()->SystemData.Provision_state == PROVISION_SUCCESS);
	if (isProvision == PROVISION_SUCCESS) {
		LOG("Provision Success\r\n");
	}
	else if(isProvision == PROVISION_PENDING)
	{
		LOG("Provision is Pending\r\n");
	}
	else
		LOG("Provision None\r\n");

#if HW_PRETEST_FW
    if((blIsBoot==0)&&!isProvision)
    {
    	while(Pair_button_read()==1)
    	{}
    	HW_Test();
    }
#endif
    while(1){
        LOG("/****************** START OF WHILE LOOP ******************/\r\n");
        Button_Handler();
        // This function is to receive the message from Gateway
        Lora_Main();
        // Process Lora
        Proc_Process();
        LOG("/****************** END OF WHILE LOOP ******************/\r\n");
    }
    return 0 ;
}
