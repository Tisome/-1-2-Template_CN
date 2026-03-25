#include "does_it_work.h"

#include "elog.h"

#include "FreeRTOS.h"
#include "task.h"

#define TASK_CLOCK_STACK_SIZE 256U
#define TASK_E2PROM_STACK_SIZE 256U
#define TASK_ELOG_STACK_SIZE 256U
#define TASK_MODBUS_STACK_SIZE 256U

#define TASK_CLOCK_PRIO 4U
#define TASK_E2PROM_PRIO 4U
#define TASK_ELOG_PRIO 3U
#define TASK_MODBUS_PRIO 5U

static int task_test(void)
{
    BaseType_t ret;

    // ret = xTaskCreate(task_clock,
    //                   "task_clock",
    //                   TASK_CLOCK_STACK_SIZE,
    //                   NULL,
    //                   TASK_CLOCK_PRIO,
    //                   NULL);
    // if (ret != pdPASS)
    // {
    //     log_e("create task_clock failed");
    //     return -1;
    // }

    // ret = xTaskCreate(task_e2prom,
    //                   "task_e2prom",
    //                   TASK_E2PROM_STACK_SIZE,
    //                   NULL,
    //                   TASK_E2PROM_PRIO,
    //                   NULL);
    // if (ret != pdPASS)
    // {
    //     log_e("create task_e2prom failed");
    //     return -1;
    // }

    ret = xTaskCreate(task_elog,
                      "task_elog",
                      TASK_ELOG_STACK_SIZE,
                      NULL,
                      TASK_ELOG_PRIO,
                      NULL);
    if (ret != pdPASS)
    {
        log_e("create task_elog failed");
        return -1;
    }

    ret = xTaskCreate(task_modbus,
                      "task_modbus",
                      TASK_MODBUS_STACK_SIZE,
                      NULL,
                      TASK_MODBUS_PRIO,
                      NULL);
    if (ret != pdPASS)
    {
        log_e("create task_modbus failed");
        return -1;
    }

    vTaskStartScheduler();

    while (1)
    {
        /* 不应该执行到这里 */
    }
}

void does_it_work(void)
{
    (void)task_test();
}