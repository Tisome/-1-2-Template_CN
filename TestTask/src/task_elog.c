#include "elog.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

void task_elog(void *p)
{
    (void)p;

    while (1)
    {
        TickType_t t1 = xTaskGetTickCount();
        vTaskDelay(pdMS_TO_TICKS(2000));
        TickType_t t2 = xTaskGetTickCount();

        log_i("alive: delta tick = %lu", (uint32_t)(t2 - t1));
    }
}