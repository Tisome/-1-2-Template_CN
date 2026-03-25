#include "does_it_work.h"

#include "elog.h"

#include "SEGGER_RTT.h"

#include "FreeRTOS.h"
#include "task.h"

static void clock_debug_dump(void)
{
    uint32_t ctl = RCU_CTL;
    uint32_t cfg0 = RCU_CFG0;

    log_i("SystemCoreClock = %lu", SystemCoreClock);
    log_i("RCU_CTL  = 0x%08lX", ctl);
    log_i("RCU_CFG0 = 0x%08lX", cfg0);

    /* 解析几个关键状态位 */
    log_i("IRC8MSTB = %lu", (ctl & RCU_CTL_IRC8MSTB) ? 1UL : 0UL);
    log_i("HXTALSTB = %lu", (ctl & RCU_CTL_HXTALSTB) ? 1UL : 0UL);
    log_i("PLLRDY   = %lu", (ctl & RCU_CTL_PLLSTB) ? 1UL : 0UL);

    /* 系统时钟切换状态 SCSS */
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

void elog_task(void *p)
{
    clock_debug_dump();
    while (1)
    {
        TickType_t t1 = xTaskGetTickCount();
        vTaskDelay(500);
        TickType_t t2 = xTaskGetTickCount();
        log_i("delta tick = %lu", t2 - t1);
    }
}

int task_test(void)
{

    xTaskCreate(elog_task, "elog", 128, NULL, 5, NULL);

    vTaskStartScheduler();

    while (1)
        ; // 不应该执行到这里
}

void does_it_work()
{
    task_test();
}