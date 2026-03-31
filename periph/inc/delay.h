#ifndef __DELAY_H
#define __DELAY_H

#include "gd32f30x.h"

#ifdef DELAY_DECLARE_ONLY
void delay_init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
#else
static __inline void delay_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static __inline void delay_us(uint32_t us)
{
    uint32_t cycles_per_us;
    uint32_t start;
    uint32_t target_cycles;

    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U)
    {
        delay_init();
    }

    cycles_per_us = SystemCoreClock / 1000000U;
    if (cycles_per_us == 0U)
    {
        cycles_per_us = 1U;
    }

    start = DWT->CYCCNT;
    target_cycles = us * cycles_per_us;
    if (target_cycles == 0U)
    {
        target_cycles = 1U;
    }

    while ((uint32_t)(DWT->CYCCNT - start) < target_cycles)
    {
    }
}

static __inline void delay_ms(uint32_t ms)
{
    while (ms > 0U)
    {
        delay_us(1000U);
        ms--;
    }
}
#endif

#endif
