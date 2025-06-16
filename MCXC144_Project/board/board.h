/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    board.h
 * @brief   Board initialization header file.
 */

/* This is an empty template for board specific configuration.*/

#ifndef _BOARD_H_
#define _BOARD_H_
#include "clock_config.h"
#include "fsl_gpio.h"
/**
 * @brief	The board name
 */
#define BOARD_NAME "board"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART0
#define BOARD_DEBUG_UART_INSTANCE 0U
#define BOARD_DEBUG_UART_CLKSRC   kCLOCK_McgIrc48MClk
#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetFreq(kCLOCK_McgIrc48MClk)
#define BOARD_UART_IRQ            LPUART0_IRQn
#define BOARD_UART_IRQ_HANDLER    LPUART0_IRQHandler

#define BOARD_DEBUG_UART_BAUDRATE 115200
/**
 * @brief 	Initialize board specific settings.
 */
void BOARD_InitDebugConsole(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
