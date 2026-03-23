#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

void led_task(void *p)
{
    while (1)
    {
        // 翻转GPIO
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    // 硬件初始化

    xTaskCreate(led_task, "led", 128, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1)
        ; // 不应该执行到这里
}
