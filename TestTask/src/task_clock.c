#include "elog.h"

#include "does_it_work.h"

#include "FreeRTOS.h"
#include "task.h"

#include "gd32f30x.h"
#include "gd32f30x_rcu.h"

static void clock_debug_dump(void)
{
    uint32_t ctl = RCU_CTL;
    uint32_t cfg0 = RCU_CFG0;

    log_i("SystemCoreClock = %lu", SystemCoreClock);
    log_i("RCU_CTL  = 0x%08lX", ctl);
    log_i("RCU_CFG0 = 0x%08lX", cfg0);

    log_i("IRC8MSTB = %lu", (ctl & RCU_CTL_IRC8MSTB) ? 1UL : 0UL);
    log_i("HXTALSTB = %lu", (ctl & RCU_CTL_HXTALSTB) ? 1UL : 0UL);
    log_i("PLLRDY   = %lu", (ctl & RCU_CTL_PLLSTB) ? 1UL : 0UL);

    switch (cfg0 & RCU_CFG0_SCSS)
    {
    case RCU_SCSS_IRC8M:
        log_i("SCSS = IRC8M");
        break;

    case RCU_SCSS_HXTAL:
        log_i("SCSS = HXTAL");
        break;

    case RCU_SCSS_PLL:
        log_i("SCSS = PLL");
        break;

    default:
        log_i("SCSS = UNKNOWN");
        break;
    }
}

void task_clock(void *p)
{
    (void)p;

    log_i("task_clock start");
    clock_debug_dump();
    log_i("task_clock done");

    vTaskDelete(NULL);
}