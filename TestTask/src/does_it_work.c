#include "does_it_work.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

void led_task(void *p)
{
    while (1)
    {
        // 翻转GPIO
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int task_test(void)
{

    xTaskCreate(led_task, "led", 128, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1)
        ; // 不应该执行到这里
}

void does_it_work()
{
    task_test();
}