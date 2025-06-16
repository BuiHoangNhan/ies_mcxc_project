/**
* @file atm90e26.h
* @brief 
*
* @details 
* @date 2025 Mar 18
* @version 1
* @author Tran Minh Sang
* Group: 
*
*/

#ifndef ATM90E26_H_
#define ATM90E26_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct{
    void (*uart_write_byte)(uint8_t *byte, uint8_t size);
    void (*uart_read_byte)(uint8_t *byte);
    void (*uart_delay)(uint32_t ms);
} uart_func_t;

/****************************Function prototypes*******************************/
extern int InitEnergyIC(void);
extern float  GetLineVoltage(void);
extern float GetLineCurrent(void);
extern float GetActivePower(void);
extern float GetFrequency(void);
extern unsigned short GetSysStatus(void);
void ATM90E26_Init(void);
/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ATM90E26_H_ */

/* End of atm90e26.h */
