/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    MCXC444_Project2.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include <3_Application/Sys_Process.h>
/* TODO: insert other include files here. */


/* TODO: insert other definitions and declarations here. */
/*1
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
//    BOARD_InitDebugConsole();
#endif
    Driver_Init();
//    PRINTF("log");
    /* Force the counter to be placed into memory. */
    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {
        Lora_Main();
#if LCD_CONTROLER
        LCD_parse();
#endif
        Proc_Process();
        /* 'Dummy' NOP to allow source level single stepping of
            tight while() loop */
    }
    return 0 ;
}
