/*
 * Modbus 命令执行任务文件。
 * 与“读/写寄存器”不同，某些线圈命令代表的是动作请求，例如：
 * 1. 清零累计量
 * 2. 启动零漂学习
 * 3. 恢复默认参数
 * 4. 软件复位
 *
 * 这些命令会先在协议层入队，再由本任务在任务上下文中执行，避免在协议解析流程中做重操作。
 */
#include "modbus_frame_process.h"
#include "modbus_map.h"
#include "data.h"
#include "gd32f30x.h"

#include "FreeRTOS.h"
#include "task.h"

#include "elog.h"

/* Modbus 动作命令执行任务入口。 */
void task_modbus_execute(void *parameter)
{
    modbus_cmd_t cmd;
    (void)parameter;

    while (1)
    {
        if (g_modbus_cmd_queue == NULL)
        {
            vTaskDelay(pdMS_TO_TICKS(10U));
            continue;
        }

        if (xQueueReceive(g_modbus_cmd_queue, &cmd, portMAX_DELAY) != pdPASS)
        {
            continue;
        }

        switch (cmd)
        {
        case MODBUS_CMD_CLEAR_TOTALIZER:
            (void)parameter_execute_action(PARAMETER_ACTION_CLEAR_TOTALIZER);
            break;

        case MODBUS_CMD_ZERO_LEARN_START:
            (void)parameter_execute_action(PARAMETER_ACTION_ZERO_LEARN_START);
            break;

        case MODBUS_CMD_SAVE_PARAMETERS:
            if (parameter_save_current() != PARAMETER_APPLY_OK)
            {
                log_e("save parameters command failed");
            }
            break;

        case MODBUS_CMD_LOAD_DEFAULT_PARAMETERS:
            (void)parameter_execute_action(PARAMETER_ACTION_LOAD_DEFAULTS);
            break;

        case MODBUS_CMD_CLEAR_ALARM:
            (void)parameter_execute_action(PARAMETER_ACTION_CLEAR_ALARM);
            break;

        case MODBUS_CMD_SOFT_RESET:
            if (parameter_storage_is_persistent() &&
                (parameter_save_current() != PARAMETER_APPLY_OK))
            {
                log_e("save parameters before reset failed");
            }
            NVIC_SystemReset();
            break;

        case MODBUS_CMD_NONE:
        default:
            break;
        }
    }
}
