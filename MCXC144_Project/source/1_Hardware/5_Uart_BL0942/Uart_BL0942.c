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
#include <2_Middle/5_Bl0942/Bl0942.h>
#include <2_Middle/4_Sensor/Sensor.h>

#if LCD_CONTROLER
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BL0942_UART          UART2
#define BL0942_UART_CLKSRC   kCLOCK_BusClk
#define BL0942_UART_CLK_FREQ CLOCK_GetFreq(kCLOCK_BusClk)
#define ECHO_BUFFER_LENGTH 1
size_t vvv = 0U;
volatile uint8_t Bl0942_buf[23];
volatile uint8_t index_bl0942 = 0;
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

volatile uint8_t g_txBuffer[ECHO_BUFFER_LENGTH] = {0};
volatile uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH] = {0};
volatile uint8_t byterx;
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
    	Bl0942_buf[index_bl0942++] = byterx;
    	if(index_bl0942==23)
    	{
            Sensor_t *Sensor = Sensor_get_value();
            Sensor->BL0942_sensor_data[Sensor->BL0942_CurChannel].status = BL0942_DONE;
    		index_bl0942 = 0;
    	}
    	UART_TransferReceiveNonBlocking(BL0942_UART, &g_uartHandle, &bl0942_receiveXfer, &vvv);
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
    Bl0942_config.baudRate_Bps = 4800;
    Bl0942_config.enableTx     = true;
    Bl0942_config.enableRx     = true;

    UART_Init(BL0942_UART, &Bl0942_config, BL0942_UART_CLK_FREQ);
    UART_TransferCreateHandle(BL0942_UART, &g_uartHandle, UART_UserCallback, NULL);
    bl0942_receiveXfer.data     = &byterx;
    bl0942_receiveXfer.dataSize = 1;
   	UART_TransferReceiveNonBlocking(BL0942_UART, &g_uartHandle, &bl0942_receiveXfer, &vvv);

}

void BL0942_writeStr(uint8_t* val, uint8_t len)
{
    bl0942_sendXfer.data        = val;
    bl0942_sendXfer.dataSize    = len;
	UART_TransferSendNonBlocking(BL0942_UART, &g_uartHandle, &bl0942_sendXfer);
}

void BL0942_readString(uint8_t* val, uint8_t len)
{
    bl0942_receiveXfer.data     = &byterx;
    bl0942_receiveXfer.dataSize = 1;
   	UART_TransferReceiveNonBlocking(BL0942_UART, &g_uartHandle, &bl0942_receiveXfer, &vvv);
}

void BL0942_delay_ms(uint32_t ms)
{
	System_DelayMs(ms);
}
#endif
