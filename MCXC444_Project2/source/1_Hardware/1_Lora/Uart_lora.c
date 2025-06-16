/*
 * Uart_lora.c
 *
 *  Created on: 21 Nov 2024
 *      Author: Huy Doan
 */


/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_lpuart_dma.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "fsl_clock.h"
#include <stdbool.h>
#include "Uart_lora.h"


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* LPUART user callback */
void LORA_UserCallback(LPUART_Type *base, lpuart_dma_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/

#define UART_RX_BUFFER_SIZE (2*DMA_RX_BUFFER_SIZE+64)
volatile uint8_t p_bEsp32RxRingBuffer[UART_RX_BUFFER_SIZE];
static volatile uint8_t* p_bEsp32RxHeadPtr = p_bEsp32RxRingBuffer;
static volatile uint8_t* p_bEsp32RxTailPtr = p_bEsp32RxRingBuffer;
static volatile uint32_t overrun = 0;

lpuart_dma_handle_t g_lpuartEdmaHandle;
dma_handle_t g_lpuartTxEdmaHandle;
dma_handle_t g_lpuartRxEdmaHandle;
AT_NONCACHEABLE_SECTION_INIT(uint8_t lora_txBuffer )= {0};
AT_NONCACHEABLE_SECTION_INIT(uint8_t lora_rxBuffer )= {0};
#if (defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) && DEMO_EDMA_HAS_CHANNEL_CONFIG)
extern edma_config_t userConfig;
#else
// edma_config_t userConfig;
#endif

lpuart_config_t lpuartConfig;
lpuart_transfer_t lora_sendXfer;
lpuart_transfer_t lora_receiveXfer;
/*******************************************************************************
 * Code
 ******************************************************************************/
/* LPUART user callback */
void LORA_UserCallback(LPUART_Type *base, lpuart_dma_handle_t *handle, status_t status, void *userData)
{
    userData = userData;



    if (kStatus_LPUART_TxIdle == status)
    {

    }

    if (kStatus_LPUART_RxIdle == status)
    {
        /* Read one byte from the receive data register */
        if (p_bEsp32RxHeadPtr != p_bEsp32RxTailPtr - 1)
        {
            if ((p_bEsp32RxHeadPtr != p_bEsp32RxRingBuffer + sizeof(p_bEsp32RxRingBuffer) - 1)
                || (p_bEsp32RxTailPtr != p_bEsp32RxRingBuffer))
            {
                *p_bEsp32RxHeadPtr++ = lora_rxBuffer;
                if (p_bEsp32RxHeadPtr >= p_bEsp32RxRingBuffer + sizeof(p_bEsp32RxRingBuffer))
                {
                  p_bEsp32RxHeadPtr = (volatile uint8_t*) p_bEsp32RxRingBuffer;
                }
            }
            else
            {
                overrun++;
            }
        }
        else
        {
            overrun++;
        }

        LPUART_TransferReceiveDMA(LORA_LPUART, &g_lpuartEdmaHandle, &lora_receiveXfer);
    }
}

/*!
 * @brief Main function
 */
void Lora_uart_init(void)
{

    /* Initialize the LPUART. */
    /*
     * lpuartConfig.baudRate_Bps = 115200U;
     * lpuartConfig.parityMode = kLPUART_ParityDisabled;
     * lpuartConfig.stopBitCount = kLPUART_OneStopBit;
     * lpuartConfig.txFifoWatermark = 0;
     * lpuartConfig.rxFifoWatermark = 0;
     * lpuartConfig.enableTx = false;
     * lpuartConfig.enableRx = false;
     */
    LPUART_GetDefaultConfig(&lpuartConfig);
    CLOCK_SetLpuart0Clock(0x1U);
    lpuartConfig.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    lpuartConfig.enableTx     = true;
    lpuartConfig.enableRx     = true;

    LPUART_Init(LORA_LPUART, &lpuartConfig, LORA_LPUART_CLK_FREQ);

#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* Init DMAMUX */
    DMAMUX_Init(EXAMPLE_LPUART_DMAMUX_BASEADDR);
    /* Set channel for LPUART */
    DMAMUX_SetSource(EXAMPLE_LPUART_DMAMUX_BASEADDR, LORA_LPUART_TX_DMA_CHANNEL, LPUART_TX_DMA_REQUEST);
    DMAMUX_SetSource(EXAMPLE_LPUART_DMAMUX_BASEADDR, LORA_LPUART_RX_DMA_CHANNEL, LPUART_RX_DMA_REQUEST);
    DMAMUX_EnableChannel(EXAMPLE_LPUART_DMAMUX_BASEADDR, LORA_LPUART_TX_DMA_CHANNEL);
    DMAMUX_EnableChannel(EXAMPLE_LPUART_DMAMUX_BASEADDR, LORA_LPUART_RX_DMA_CHANNEL);
#endif
    /* Init the EDMA module */
#if (!defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) || (defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) && !DEMO_EDMA_HAS_CHANNEL_CONFIG))
#endif
    DMA_Init(EXAMPLE_LPUART_DMA_BASEADDR);
    DMA_CreateHandle(&g_lpuartTxEdmaHandle, EXAMPLE_LPUART_DMA_BASEADDR, LORA_LPUART_TX_DMA_CHANNEL);
    DMA_CreateHandle(&g_lpuartRxEdmaHandle, EXAMPLE_LPUART_DMA_BASEADDR, LORA_LPUART_RX_DMA_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(LORA_LPUART_DMA_BASEADDR, LORA_LPUART_TX_DMA_CHANNEL, LORA_LPUART_TX_EDMA_CHANNEL);
    EDMA_SetChannelMux(LORA_LPUART_DMA_BASEADDR, LORA_LPUART_RX_DMA_CHANNEL, LORA_LPUART_RX_EDMA_CHANNEL);
#endif
    /* Create LPUART DMA handle. */
    LPUART_TransferCreateHandleDMA(LORA_LPUART, &g_lpuartEdmaHandle, LORA_UserCallback, NULL, &g_lpuartTxEdmaHandle,
                                    &g_lpuartRxEdmaHandle);

    /* Start to echo. */
    lora_sendXfer.data        = &lora_txBuffer;
    lora_sendXfer.dataSize    = ECHO_BUFFER_LENGTH;
    lora_receiveXfer.data     = &lora_rxBuffer;
    lora_receiveXfer.dataSize = ECHO_BUFFER_LENGTH;
    LPUART_TransferReceiveDMA(LORA_LPUART, &g_lpuartEdmaHandle, &lora_receiveXfer);


}

void uart_put_bytes(uint8_t*cmdData, uint16_t dataSize)
{
    lora_sendXfer.data        = cmdData;
    lora_sendXfer.dataSize    = dataSize;
	LPUART_TransferSendDMA(LORA_LPUART, &g_lpuartEdmaHandle, &lora_sendXfer);
}

uint32_t uart_get_char(uint8_t * bChar)
{
    if (p_bEsp32RxHeadPtr != p_bEsp32RxTailPtr)
    {
        *bChar = *p_bEsp32RxTailPtr++;
        if (p_bEsp32RxTailPtr >= p_bEsp32RxRingBuffer + sizeof(p_bEsp32RxRingBuffer))
        {
            p_bEsp32RxTailPtr = (volatile uint8_t*) p_bEsp32RxRingBuffer;
        }
        return 1;
    }
    return 0;
}

uint8_t uart_peek_char(uint8_t *c)
{
    if (p_bEsp32RxHeadPtr != p_bEsp32RxTailPtr)
    {
        *c = *p_bEsp32RxTailPtr;
        return 1;
    }
    return 0;
}
void uart_drv_buffer_clear(void)
{

    memset(p_bEsp32RxRingBuffer, 0x00, sizeof p_bEsp32RxRingBuffer);

    p_bEsp32RxHeadPtr = p_bEsp32RxRingBuffer;
    p_bEsp32RxTailPtr = p_bEsp32RxRingBuffer;
    overrun = 0;
}

void Lora_uart_main(void)
{

}

