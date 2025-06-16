#ifndef __2_LCD_H
#define __2_LCD_H
#include <stdint.h>
#include <stdbool.h>
#if LCD_CONTROLER
// void Lcd_uart_init(void);
// void LCD_Sendcmd(uint8_t*cmdData, uint16_t dataSize);
#elif NODE_NEMA
void Power_uart_init(void);

void POWER_Sendcmd(uint8_t *cmdData, uint8_t size);

void Power_uart_get_byte(uint8_t * bChar);
#endif

#endif
