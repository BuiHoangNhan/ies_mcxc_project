#ifndef __TIMER_LIB_H
#define __TIMER_LIB_H
#include <stdint.h>

typedef struct
{
  // uint32_t start;
  uint64_t totalTickMs;
} timer_t;

void Timer_Init(void);

void System_DelayMs(uint32_t ms);
void Timer_Create(timer_t* t, uint64_t intervalMs);
uint64_t Timer_Timeout(timer_t* t);
#endif /*__TIMER_H*/
