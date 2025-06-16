#ifndef __TIMER_LIB_H
#define __TIMER_LIB_H
#include <stdint.h>

typedef struct
{
  // uint32_t start;
  uint32_t totalTickMs;
} timer_t;

void Timer_Init(void);

void System_DelayMs(uint32_t ms);
void Timer_Create(timer_t* t, uint32_t intervalMs);
uint32_t Timer_Timeout(timer_t* t);

#endif /*__TIMER_H*/
