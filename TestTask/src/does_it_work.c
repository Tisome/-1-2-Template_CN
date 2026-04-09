#include "does_it_work.h"

#include "app_config.h"
#include "bsp_key.h"
#include "data.h"
#include "elog.h"
#include "freertos_resources.h"
#include "lvgl_app_test.h"
#include "modbus_execute.h"
#include "modbus_frame_process.h"
#include "modbus_parse.h"

#include "FreeRTOS.h"
#include "task.h"

#define TASK_ELOG_STACK_SIZE           384U
#define TASK_SPI_RX_STACK_SIZE         384U
#define TASK_MODBUS_PARSE_STACK_SIZE   768U
#define TASK_MODBUS_EXECUTE_STACK_SIZE 384U
#define TASK_KEY_STACK_SIZE            384U
#define TASK_FAKE_DATA_STACK_SIZE      384U
#define TASK_ALGORITHM_STACK_SIZE      512U
#define TASK_LVGL_TEST_STACK_SIZE      1024U

#define TASK_ELOG_PRIO           3U
#define TASK_SPI_RX_PRIO         7U
#define TASK_MODBUS_PARSE_PRIO   5U
#define TASK_MODBUS_EXECUTE_PRIO 5U
#define TASK_KEY_PRIO            5U
#define TASK_FAKE_DATA_PRIO      4U
#define TASK_ALGORITHM_PRIO      5U
#define TASK_LVGL_TEST_PRIO      6U

TaskHandle_t task_spi_rx_handler = NULL;
TaskHandle_t task_modbus_handler = NULL;

static int create_task(TaskFunction_t entry,
                       const char *name,
                       uint16_t stack_words,
                       UBaseType_t priority,
                       TaskHandle_t *handle)
{
    BaseType_t ret = xTaskCreate(entry,
                                 name,
                                 stack_words,
                                 NULL,
                                 priority,
                                 handle);
    if (ret != pdPASS)
    {
        log_e("create %s failed", name);
        return -1;
    }

    return 0;
}

static int task_test(void)
{
#if APP_ENABLE_MEASURE_FPGA_SPI
    if (create_task(task_spi_rx,
                    "task_spi_rx",
                    TASK_SPI_RX_STACK_SIZE,
                    TASK_SPI_RX_PRIO,
                    &task_spi_rx_handler) != 0)
    {
        return -1;
    }
#endif

#if APP_ENABLE_MEASURE_FAKE_DATA
    if (create_task(task_fake_data,
                    "task_fake_data",
                    TASK_FAKE_DATA_STACK_SIZE,
                    TASK_FAKE_DATA_PRIO,
                    NULL) != 0)
    {
        return -1;
    }
#endif

#if APP_ENABLE_ALGORITHM_TASK
    if (create_task(task_algorithm,
                    "task_algorithm",
                    TASK_ALGORITHM_STACK_SIZE,
                    TASK_ALGORITHM_PRIO,
                    NULL) != 0)
    {
        return -1;
    }
#endif

#if APP_ENABLE_KEY_TASK
    if (create_task(task_key,
                    "task_key",
                    TASK_KEY_STACK_SIZE,
                    TASK_KEY_PRIO,
                    &task_key_handle) != 0)
    {
        return -1;
    }
#endif

#if APP_ENABLE_GUI_TASK
    if (create_task(task_lvgl_test,
                    "task_lvgl_test",
                    TASK_LVGL_TEST_STACK_SIZE,
                    TASK_LVGL_TEST_PRIO,
                    NULL) != 0)
    {
        return -1;
    }
#endif

#if APP_ENABLE_MODBUS_RUNTIME
    if (create_task(task_modbus_parse,
                    "task_modbus",
                    TASK_MODBUS_PARSE_STACK_SIZE,
                    TASK_MODBUS_PARSE_PRIO,
                    &task_modbus_handler) != 0)
    {
        return -1;
    }

    if (create_task(task_modbus_execute,
                    "task_modbus_execute",
                    TASK_MODBUS_EXECUTE_STACK_SIZE,
                    TASK_MODBUS_EXECUTE_PRIO,
                    NULL) != 0)
    {
        return -1;
    }
#endif

#if APP_ENABLE_ELOG_TASK
    if (create_task(task_elog,
                    "task_elog",
                    TASK_ELOG_STACK_SIZE,
                    TASK_ELOG_PRIO,
                    NULL) != 0)
    {
        return -1;
    }
#endif

    vTaskStartScheduler();

    while (1)
    {
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
    freertos_resources_init();
    parameter_init();

#if APP_ENABLE_MODBUS_RUNTIME
    init_modbus_data();
#endif

    (void)task_test();
}
