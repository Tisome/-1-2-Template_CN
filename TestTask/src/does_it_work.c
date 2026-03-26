#include "does_it_work.h"

#include "elog.h"

#include "FreeRTOS.h"
#include "task.h"

#define TASK_CLOCK_STACK_SIZE 256U
#define TASK_E2PROM_STACK_SIZE 256U
#define TASK_ELOG_STACK_SIZE 256U
#define TASK_MODBUS_STACK_SIZE 1024U
#define TASK_SPI_RX_STACK_SIZE 256U

#define TASK_CLOCK_PRIO 4U
#define TASK_E2PROM_PRIO 4U
#define TASK_ELOG_PRIO 3U
#define TASK_MODBUS_PRIO 5U
#define TASK_SPI_RX_PRIO 5U

TaskHandle_t task_spi_rx_handler = NULL;
TaskHandle_t task_modbus_handler = NULL;

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

    // ret = xTaskCreate(task_modbus,
    //                   "task_modbus",
    //                   TASK_MODBUS_STACK_SIZE,
    //                   NULL,
    //                   TASK_MODBUS_PRIO,
    //                   &task_modbus_handler);
    // if (ret != pdPASS)
    // {
    //     log_e("create task_modbus failed");
    //     return -1;
    // }

    ret = xTaskCreate(task_spi_rx,
                      "taske_spi_rx",
                      TASK_SPI_RX_STACK_SIZE,
                      NULL,
                      TASK_SPI_RX_PRIO,
                      &task_spi_rx_handler);
    if (ret != pdPASS)
    {
        log_e("create task_spi_rx failed");
        return -1;
    }

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

    vTaskStartScheduler();

    while (1)
    {
        /* 不应该执行到这里 */
    }
}

TaskHandle_t get_spi_rx_task_handle(void)
{
    return task_spi_rx_handler;
}

TaskHandle_t get_modbus_task_handle(void)
{
    return task_modbus_handler;
}

void does_it_work(void)
{
    (void)task_test();
}