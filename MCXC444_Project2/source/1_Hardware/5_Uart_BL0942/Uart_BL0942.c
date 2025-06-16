#include "Uart_BL0942.h"
/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_uart.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BL0942_UART          UART2
#define BL0942_UART_CLKSRC   kCLOCK_BusClk
#define BL0942_UART_CLK_FREQ CLOCK_GetFreq(kCLOCK_BusClk)
#define ECHO_BUFFER_LENGTH 1

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* UART user callback */
void UART_UserCallback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uart_handle_t g_uartHandle;

uint8_t g_tipString1[] =
    "Uart interrupt example\r\nBoard receives 8 characters then sends them out\r\nNow please input:\r\n";

uint8_t g_txBuffer[ECHO_BUFFER_LENGTH] = {0};
uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH] = {0};

uart_config_t Bl0942_config;
uart_transfer_t bl0942_sendXfer;
uart_transfer_t bl0942_receiveXfer;
/*******************************************************************************
 * Code
 ******************************************************************************/

void UART2_FLEXIO_IRQHandler(void)
{
    UART_TransferHandleIRQ(UART2, &g_uartHandle);
}
/* UART user callback */
void UART_UserCallback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_UART_TxIdle == status)
    {

    }

    if (kStatus_UART_RxIdle == status)
    {

    }
}

/*!
 * @brief Main function
 */
void BL0942_UART_Init(void)
{
    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUART_ParityDisabled;
     * config.stopBitCount = kUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 1;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    UART_GetDefaultConfig(&Bl0942_config);
    Bl0942_config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    Bl0942_config.enableTx     = true;
    Bl0942_config.enableRx     = true;

    UART_Init(BL0942_UART, &Bl0942_config, BL0942_UART_CLK_FREQ);
    UART_TransferCreateHandle(BL0942_UART, &g_uartHandle, UART_UserCallback, NULL);

}

void BL0942_writeStr(uint8_t* val, uint8_t len)
{
    bl0942_sendXfer.data        = val;
    bl0942_sendXfer.dataSize    = len;
	UART_TransferSendNonBlocking(BL0942_UART, &g_uartHandle, &bl0942_sendXfer);
}

void BL0942_readString(uint8_t* val, uint8_t len)
{
    bl0942_receiveXfer.data     = val;
    bl0942_receiveXfer.dataSize = len;
   	UART_TransferReceiveNonBlocking(BL0942_UART, &g_uartHandle, &bl0942_receiveXfer, NULL);
}

void BL0942_delay_ms(uint32_t ms)
{
	System_DelayMs(ms);
}
