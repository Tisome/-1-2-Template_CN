#include "does_it_work.h"

#include "elog.h"

#include "at24cxx_handler.h"

#include "SEGGER_RTT.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#define E2PROM_TEST_ADDR 0x0040U
#define E2PROM_TEST_LEN 16U

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

static void e2prom_rw_test(void)
{
    uint8_t write_buf[E2PROM_TEST_LEN] = {0};
    uint8_t read_buf[E2PROM_TEST_LEN] = {0};

    for (uint32_t i = 0; i < E2PROM_TEST_LEN; i++)
    {
        write_buf[i] = (uint8_t)(0xA0U + i);
    }

    eeprom_status_t write_status = eeprom_write_sync(E2PROM_TEST_ADDR,
                                                     write_buf,
                                                     E2PROM_TEST_LEN);
    if (write_status != E2PROM_OK)
    {
        log_e("e2prom write failed, status=%d", write_status);
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(10));

    eeprom_status_t read_status = eeprom_read_sync(E2PROM_TEST_ADDR,
                                                   read_buf,
                                                   E2PROM_TEST_LEN);
    if (read_status != E2PROM_OK)
    {
        log_e("e2prom read failed, status=%d", read_status);
        return;
    }

    if (memcmp(write_buf, read_buf, E2PROM_TEST_LEN) != 0)
    {
        log_e("e2prom data mismatch");
        for (uint32_t i = 0; i < E2PROM_TEST_LEN; i++)
        {
            log_i("idx=%lu write=0x%02X read=0x%02X", i, write_buf[i], read_buf[i]);
        }
        return;
    }

    log_i("e2prom rw test pass, addr=0x%04X len=%u", E2PROM_TEST_ADDR, E2PROM_TEST_LEN);
}

void elog_task(void *p)
{
    (void)p;
    clock_debug_dump();
    e2prom_rw_test();
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

    xTaskCreate(elog_task, "elog", 256, NULL, 5, NULL);

    vTaskStartScheduler();

    while (1)
        ; // 不应该执行到这里
}

void does_it_work()
{
    task_test();
}