#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>

typedef struct
{
    uint32_t fault_code;
    uint32_t task_handle_addr;
    char task_name[configMAX_TASK_NAME_LEN];
} rtos_fault_diag_t;

/* Visible in Ozone even after the CPU is trapped in a fault hook. */
volatile rtos_fault_diag_t g_rtos_fault_diag;

static void hook_copy_name(volatile char *dst, uint32_t dst_size, const char *src)
{
    uint32_t i;

    if ((dst == NULL) || (dst_size == 0U))
    {
        return;
    }

    if (src == NULL)
    {
        dst[0] = '\0';
        return;
    }

    for (i = 0U; (i + 1U) < dst_size; i++)
    {
        char ch = src[i];
        dst[i] = ch;

        if (ch == '\0')
        {
            return;
        }
    }

    dst[dst_size - 1U] = '\0';
}

void vApplicationMallocFailedHook(void)
{
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    const char *task_name = NULL;

    if (current_task != NULL)
    {
        task_name = pcTaskGetName(current_task);
    }

    g_rtos_fault_diag.fault_code = 1U;
    g_rtos_fault_diag.task_handle_addr = (uint32_t)current_task;
    hook_copy_name(g_rtos_fault_diag.task_name,
                   (uint32_t)sizeof(g_rtos_fault_diag.task_name),
                   task_name);

    taskDISABLE_INTERRUPTS();
    while (1)
        ;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    g_rtos_fault_diag.fault_code = 2U;
    g_rtos_fault_diag.task_handle_addr = (uint32_t)xTask;
    hook_copy_name(g_rtos_fault_diag.task_name,
                   (uint32_t)sizeof(g_rtos_fault_diag.task_name),
                   pcTaskName);

    printf("Stack overflow in task: %s\r\n", pcTaskName);

    taskDISABLE_INTERRUPTS();
    while (1)
        ;
}
