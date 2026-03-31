#define DELAY_DECLARE_ONLY
#include "delay.h"

static uint32_t s_cycles_per_us = 0U;

void delay_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    s_cycles_per_us = SystemCoreClock / 1000000U;
    if (s_cycles_per_us == 0U)
    {
        s_cycles_per_us = 1U;
    }
}

void delay_us(uint32_t us)
{
    uint32_t start;
    uint32_t target_cycles;

    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U)
    {
        delay_init();
    }

    start = DWT->CYCCNT;
    target_cycles = us * s_cycles_per_us;
    if (target_cycles == 0U)
    {
        target_cycles = 1U;
    }

    while ((uint32_t)(DWT->CYCCNT - start) < target_cycles)
    {
    }
}

void delay_ms(uint32_t ms)
{
    while (ms > 0U)
    {
        delay_us(1000U);
        ms--;
    }
}
