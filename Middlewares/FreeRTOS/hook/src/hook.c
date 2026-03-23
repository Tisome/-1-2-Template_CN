#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    while (1)
        ;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("Stack overflow in task: %s\r\n", pcTaskName);

    taskDISABLE_INTERRUPTS();
    while (1)
        ;
}