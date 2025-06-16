#ifndef __UART_BL0942_H_
#define __UART_BL0942_H_

#include <stdint.h>
#include <stdbool.h>
void BL0942_UART_Init(void);

void BL0942_writeStr(uint8_t* val, uint8_t len) ;

void BL0942_readString(uint8_t* val, uint8_t len);

void BL0942_delay_ms(uint32_t ms);

#endif