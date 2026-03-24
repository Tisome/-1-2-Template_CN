#include "does_it_work.h"

#include "elog.h"

#include "FreeRTOS.h"
#include "task.h"

void elog_task(void *p)
{
    while (1)
    {
        log_i("hello world");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int task_test(void)
{

    xTaskCreate(elog_task, "elog", 128, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1)
        ; // 不应该执行到这里
}

void does_it_work()
{
    task_test();
}