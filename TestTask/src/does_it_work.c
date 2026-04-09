/*
 * 测试任务总装配文件。
 * 本文件负责创建当前测试固件会启动的 FreeRTOS 任务，并决定哪些子系统参与联调：
 * 1. Modbus 解析与命令执行
 * 2. LVGL 界面任务
 * 3. 按键任务
 * 4. 假数据与算法处理链路
 *
 * 若后续切换到真实 SPI 或 EEPROM 联调，可优先从本文件调整任务开关、优先级和栈大小。
 */
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

#if ENABLE_FPGA_SPI_COMM_TEST
#define ENABLE_MODBUS_PARSE_TASK        0U
#define ENABLE_MODBUS_EXECUTE_TASK      0U
#define ENABLE_SPI_COMM_TEST_TASK       1U
#define ENABLE_MENU_SIM_DATA_PIPELINE   0U
#define ENABLE_ALGORITHM_TASK           0U
#define ENABLE_LVGL_TEST_TASK           0U
#define ENABLE_KEY_TASK                 0U
#else
#define ENABLE_MODBUS_PARSE_TASK        1U
#define ENABLE_MODBUS_EXECUTE_TASK      1U
#define ENABLE_SPI_COMM_TEST_TASK       0U
#define ENABLE_MENU_SIM_DATA_PIPELINE   1U
#define ENABLE_ALGORITHM_TASK           1U
#define ENABLE_LVGL_TEST_TASK           1U
#define ENABLE_KEY_TASK                 1U
#endif

#define TASK_CLOCK_STACK_SIZE 256U
#define TASK_E2PROM_STACK_SIZE 256U
#define TASK_ELOG_STACK_SIZE 384U
#define TASK_MODBUS_PARSE_STACK_SIZE 768U
#define TASK_MODBUS_EXECUTE_STACK_SIZE 384U
#define TASK_SPI_RX_STACK_SIZE 256U
#define TASK_KEY_STACK_SIZE 384U
#define TASK_KEY_TEST_STACK_SIZE 512U
#define TASK_FAKE_DATA_STACK_SIZE 384U
#define TASK_ALGORITHM_STACK_SIZE 512U
#define TASK_LVGL_TEST_STACK_SIZE 1024U

#define TASK_CLOCK_PRIO 4U
#define TASK_E2PROM_PRIO 4U
#define TASK_ELOG_PRIO 3U
#define TASK_MODBUS_PARSE_PRIO 5U
#define TASK_MODBUS_EXECUTE_PRIO 5U
#define TASK_SPI_RX_PRIO 7U
#define TASK_KEY_PRIO 5U
#define TASK_KEY_TEST_PRIO 4U
#define TASK_FAKE_DATA_PRIO 4U
#define TASK_ALGORITHM_PRIO 5U
#define TASK_LVGL_TEST_PRIO 6U

TaskHandle_t task_spi_rx_handler = NULL;
TaskHandle_t task_modbus_handler = NULL;

/* 按当前测试配置创建任务，并最终启动调度器。 */
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

#if ENABLE_MODBUS_PARSE_TASK
    ret = xTaskCreate(task_modbus_parse,
                      "task_modbus",
                      TASK_MODBUS_PARSE_STACK_SIZE,
                      NULL,
                      TASK_MODBUS_PARSE_PRIO,
                      &task_modbus_handler);
    if (ret != pdPASS)
    {
        log_e("create task_modbus failed");
        return -1;
    }
#endif

#if ENABLE_SPI_COMM_TEST_TASK
    ret = xTaskCreate(task_spi_rx,
                      "task_spi_rx",
                      TASK_SPI_RX_STACK_SIZE,
                      NULL,
                      TASK_SPI_RX_PRIO,
                      &task_spi_rx_handler);
    if (ret != pdPASS)
    {
        log_e("create task_spi_rx failed");
        return -1;
    }
#endif

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

#if ENABLE_LVGL_TEST_TASK
    ret = xTaskCreate(task_lvgl_test,
                      "task_lvgl_test",
                      TASK_LVGL_TEST_STACK_SIZE,
                      NULL,
                      TASK_LVGL_TEST_PRIO,
                      NULL);
    if (ret != pdPASS)
    {
        log_e("create task_lvgl_test failed");
        return -1;
    }
#endif

#if ENABLE_KEY_TASK
    ret = xTaskCreate(task_key,
                      "task_key",
                      TASK_KEY_STACK_SIZE,
                      NULL,
                      TASK_KEY_PRIO,
                      &task_key_handle);
    if (ret != pdPASS)
    {
        log_e("create task_key failed");
        return -1;
    }
#endif

#if ENABLE_MODBUS_EXECUTE_TASK
    ret = xTaskCreate(task_modbus_execute,
                      "task_modbus_execute",
                      TASK_MODBUS_EXECUTE_STACK_SIZE,
                      NULL,
                      TASK_MODBUS_EXECUTE_PRIO,
                      NULL);
    if (ret != pdPASS)
    {
        log_e("create task_modbus_execute failed");
        return -1;
    }
#endif

#if ENABLE_MENU_SIM_DATA_PIPELINE
    ret = xTaskCreate(task_fake_data,
                      "task_fake_data",
                      TASK_FAKE_DATA_STACK_SIZE,
                      NULL,
                      TASK_FAKE_DATA_PRIO,
                      NULL);
    if (ret != pdPASS)
    {
        log_e("create task_fake_data failed");
        return -1;
    }
#endif

#if ENABLE_ALGORITHM_TASK
    ret = xTaskCreate(task_algorithm,
                      "task_algorithm",
                      TASK_ALGORITHM_STACK_SIZE,
                      NULL,
                      TASK_ALGORITHM_PRIO,
                      NULL);
    if (ret != pdPASS)
    {
        log_e("create task_algorithm failed");
        return -1;
    }
#endif

    vTaskStartScheduler();

    while (1)
    {
        /* 不应该执行到这里 */
    }
}

/* 返回真实 SPI 接收任务句柄，供中断或其他模块在需要时唤醒该任务。 */
TaskHandle_t get_spi_rx_task_handle(void)
{
    return task_spi_rx_handler;
}

/* 返回真实 SPI 接收任务句柄，供中断或其他模块在需要时唤醒该任务。 */
TaskHandle_t get_modbus_task_handle(void)
{
    return task_modbus_handler;
}

/*
 * 系统测试入口。
 * 调用顺序是：
 * 1. 初始化 FreeRTOS 共享资源
 * 2. 初始化参数与 Modbus 映射
 * 3. 创建并启动测试任务
 */
void does_it_work(void)
{
    freertos_resources_init();
    parameter_init();
    init_modbus_data();
    (void)task_test();
}
