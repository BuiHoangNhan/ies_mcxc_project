#ifndef __UART_LORA_H
#define __UART_LORA_H
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LORA_LPUART                 LPUART0
#define LORA_LPUART_CLKSRC          BOARD_DEBUG_UART_CLKSRC
#define LORA_LPUART_CLK_FREQ        CLOCK_GetFreq(LORA_LPUART_CLKSRC)
#define LORA_LPUART_TX_DMA_CHANNEL       0U
#define LORA_LPUART_RX_DMA_CHANNEL       1U
#define EXAMPLE_LPUART_DMAMUX_BASEADDR DMAMUX0
#define EXAMPLE_LPUART_DMA_BASEADDR    DMA0
#define LPUART_TX_DMA_REQUEST          kDmaRequestMux0LPUART0Tx
#define LPUART_RX_DMA_REQUEST          kDmaRequestMux0LPUART0Rx
#define LORA_LPUART_DMA_BASEADDR DMA0
#define ECHO_BUFFER_LENGTH 1
#define DMA_RX_BUFFER_SIZE 128

void Lora_uart_main(void);
void Lora_uart_init(void);
void uart_put_bytes(uint8_t*cmdData, uint16_t dataSize);
void uart_drv_buffer_clear(void);

#endif //__UART_LORA_H