#include "modbus_frame_process.h"
#include "modbus_map.h"
#include "data.h"
#include "gd32f30x.h"

#include "FreeRTOS.h"
#include "task.h"

#include "elog.h"

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
