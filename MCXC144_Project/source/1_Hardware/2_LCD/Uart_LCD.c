/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_lpuart.h"

#include "fsl_clock.h"
#include <stdbool.h>
#if LCD_CONTROLER
#include <2_Middle/2_LCD/LCD_Dwin.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LCD_LPUART            LPUART1
#define LCD_LPUART_CLKSRC   BOARD_DEBUG_UART_CLKSRC
#define LCD_LPUART_CLK_FREQ CLOCK_GetFreq(BOARD_DEBUG_UART_CLKSRC)

#define  LCD_UART_BAUDRATE 115200u

#define ECHO_BUFFER_SIZE    12U
extern  volatile uint8_t blSend;
volatile uint8_t index_lcdbuf = 0;
static volatile uint8_t lcd_rxBuffer = 0;
static uint8_t lcd_txBuffer; /* Buffer for receive data to echo. */
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
    	readDataDWIN.uartBuffer[index_lcdbuf++] = lcd_rxBuffer;
		if(index_lcdbuf==DWIN_UART_BUFFER_SIZE)
        {
			index_lcdbuf =0;
            parsingDWIN();
            LCD_parse();
            readDataDWIN.uartCnt = 0;
        }
        else if(readDataDWIN.uartBuffer[index_lcdbuf-2]==0x4F&&readDataDWIN.uartBuffer[index_lcdbuf-1]==0x4B)
		{
            blSend = 0;
            index_lcdbuf =0;
            readDataDWIN.uartCnt = 0;
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
    lcd_receiveXfer.data     = &lcd_rxBuffer;
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
#elif NODE_NEMA

#define POWER_MONITOR_LPUART            LPUART1
#define POWER_MONITOR_LPUART_CLKSRC   BOARD_DEBUG_UART_CLKSRC
#define POWER_MONITOR_LPUART_CLK_FREQ CLOCK_GetFreq(BOARD_DEBUG_UART_CLKSRC)

#define  POWER_MONITOR_UART_BAUDRATE 9600u

volatile uint8_t rxbytePOWER;

volatile uint8_t p_powerRxRingBuffer[128];
static volatile uint8_t* p_powerRxHeadPtr = p_powerRxRingBuffer;
static volatile uint8_t* p_powerRxTailPtr = p_powerRxRingBuffer;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* LPUART user callback */
void POWER_MONITOR_UserCallback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData);
/*******************************************************************************
 * Variables
 ******************************************************************************/
lpuart_handle_t g_lpuartHandle;

volatile uint8_t Power_rxBuffer; /* Buffer for receive data to echo. */
volatile uint8_t Power_txBuffer; /* Buffer for send data to echo. */

volatile bool txOnGoing              = false;
volatile bool rxOnGoing              = false;

lpuart_config_t config;
volatile lpuart_transfer_t Power_sendXfer;
volatile lpuart_transfer_t Power_receiveXfer;
lpuart_transfer_t xfer;
size_t receivedBytes = 0U;
void POWER_UserCallback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_LPUART_TxIdle == status)
    {

    }

    if (kStatus_LPUART_RxIdle == status)
    {
        if (p_powerRxHeadPtr != p_powerRxTailPtr - 1)
        {
            if ((p_powerRxHeadPtr != p_powerRxRingBuffer + sizeof(p_powerRxRingBuffer) - 1)
                || (p_powerRxTailPtr != p_powerRxRingBuffer))
            {
                *p_powerRxHeadPtr++ = rxbytePOWER;
                if (p_powerRxHeadPtr >= p_powerRxRingBuffer + sizeof(p_powerRxRingBuffer))
                {
                    p_powerRxHeadPtr = (volatile uint8_t*) p_powerRxRingBuffer;
                }
            }
        }
        LPUART_TransferReceiveNonBlocking(POWER_MONITOR_LPUART, &g_lpuartHandle, &Power_receiveXfer, &receivedBytes);
    }
}

void Power_uart_init(void)
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
    config.baudRate_Bps = POWER_MONITOR_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    LPUART_Init(POWER_MONITOR_LPUART, &config, POWER_MONITOR_LPUART_CLK_FREQ);
    LPUART_TransferCreateHandle(POWER_MONITOR_LPUART, &g_lpuartHandle, POWER_UserCallback, NULL);

    /* Start to echo. */
    Power_sendXfer.data        = &Power_txBuffer;
    Power_sendXfer.dataSize    = 1;
    Power_receiveXfer.data     = &rxbytePOWER;
    Power_receiveXfer.dataSize = 1;
    txOnGoing     = true;

    LPUART_TransferReceiveNonBlocking(POWER_MONITOR_LPUART, &g_lpuartHandle, &Power_receiveXfer, &receivedBytes);
}
void POWER_Sendcmd(uint8_t *cmdData, uint8_t size)
{
    Power_sendXfer.data        = cmdData;
    Power_sendXfer.dataSize    = size;
    LPUART_TransferSendNonBlocking(POWER_MONITOR_LPUART, &g_lpuartHandle, &Power_sendXfer);
}
void Power_uart_get_byte(uint8_t * bChar)
{
    if (p_powerRxHeadPtr != p_powerRxTailPtr)
    {
        *bChar = *p_powerRxTailPtr++;
        if (p_powerRxTailPtr >= p_powerRxRingBuffer + sizeof(p_powerRxRingBuffer))
        {
            p_powerRxTailPtr = (volatile uint8_t*) p_powerRxRingBuffer;
        }
        return 1;
    }
    return 0;
}
#endif
