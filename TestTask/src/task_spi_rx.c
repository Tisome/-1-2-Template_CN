#include "does_it_work.h"

#include "FreeRTOS.h"
#include "task.h"

#include "elog.h"

void task_spi_rx(void *p)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        log_i("start spi rx");
    }
}