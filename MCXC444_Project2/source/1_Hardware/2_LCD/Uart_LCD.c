/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if LCD_CONTROLER
#include "pin_mux.h"
#include "board.h"
#include "fsl_lpuart.h"

#include "fsl_clock.h"
#include <stdbool.h>
#include <2_Middle/2_LCD/LCD_Dwin.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LCD_LPUART            LPUART1
#define LCD_LPUART_CLKSRC   BOARD_DEBUG_UART_CLKSRC
#define LCD_LPUART_CLK_FREQ CLOCK_GetFreq(BOARD_DEBUG_UART_CLKSRC)

#define  LCD_UART_BAUDRATE 115200u

#define ECHO_BUFFER_SIZE    12U
static volatile uint8_t index = 0;
static volatile uint8_t rxbyteLCD = 0;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* LPUART user callback */
void LCD_UserCallback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData);
extern struct readDataDWIN_P readDataDWIN;
/*******************************************************************************
 * Variables
 ******************************************************************************/
lpuart_handle_t g_lpuartHandle;

uint8_t lcd_rxBuffer; /* Buffer for receive data to echo. */
uint8_t lcd_txBuffer; /* Buffer for send data to echo. */

volatile bool txOnGoing              = false;
volatile bool rxOnGoing              = false;

lpuart_config_t config;
volatile lpuart_transfer_t lcd_sendXfer;
lpuart_transfer_t lcd_receiveXfer;
lpuart_transfer_t xfer;
size_t receivedBytes = 0U;
/*******************************************************************************
 * Code
 ******************************************************************************/

/* LPUART user callback */
void LCD_UserCallback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_LPUART_TxIdle == status)
    {

    }

    if (kStatus_LPUART_RxIdle == status)
    {
    	readDataDWIN.uartBuffer[index++] = rxbyteLCD;
		if(index==DWIN_UART_BUFFER_SIZE||(readDataDWIN.uartBuffer[index-2]==0x4F&&readDataDWIN.uartBuffer[index-1]==0x4B))
		{
			index =0;
	    	parsingDWIN();
		}
     LPUART_TransferReceiveNonBlocking(LCD_LPUART, &g_lpuartHandle, &lcd_receiveXfer, &receivedBytes);
    }
}


void Lcd_uart_init(void)
{

    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kLPUART_ParityDisabled;
     * config.stopBitCount = kLPUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 0;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    CLOCK_SetLpuart1Clock(0x1U);
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = LCD_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    LPUART_Init(LCD_LPUART, &config, LCD_LPUART_CLK_FREQ);
    LPUART_TransferCreateHandle(LCD_LPUART, &g_lpuartHandle, LCD_UserCallback, NULL);
    //LPUART_TransferStartRingBuffer(LCD_LPUART, &g_lpuartHandle, g_rxRingBuffer, RX_RING_BUFFER_SIZE);

    /* Start to echo. */
    lcd_sendXfer.data        = &lcd_txBuffer;
    lcd_sendXfer.dataSize    = 9;
    lcd_receiveXfer.data     = &rxbyteLCD;
    lcd_receiveXfer.dataSize = 1;
    txOnGoing     = true;

    LPUART_TransferReceiveNonBlocking(LCD_LPUART, &g_lpuartHandle, &lcd_receiveXfer, &receivedBytes);
}

void LCD_Sendcmd(uint8_t*cmdData, uint16_t dataSize)
{
    lcd_sendXfer.data        = cmdData;
    lcd_sendXfer.dataSize    = dataSize;
 LPUART_TransferSendNonBlocking(LCD_LPUART, &g_lpuartHandle, &lcd_sendXfer);
}
#endif
