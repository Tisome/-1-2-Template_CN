#include "modbus_frame_process.h"
#include "modbus_map.h"

// #include "algorithm_flow.h"
// #include "at24cxx_handler.h"
// #include "data.h"

#include "FreeRTOS.h"
#include "task.h"

#include "elog.h"

void task_modbus_execute(void *parameter)
{
    modbus_cmd_t cmd;

    (void)parameter;

    // while (1)
    // {
    //     if (g_modbus_cmd_queue == NULL)
    //     {
    //         vTaskDelay(pdMS_TO_TICKS(10));
    //         continue;
    //     }

    //     if (xQueueReceive(g_modbus_cmd_queue, &cmd, portMAX_DELAY) != pdPASS)
    //     {
    //         continue;
    //     }

    //     switch (cmd)
    //     {
    //     case MODBUS_CMD_CLEAR_TOTALIZER:
    //         g_algo_state.q_total_m3 = 0.0;
    //         g_algo_out.flow_rate_total = 0.0;
    //         update_input_registers();
    //         break;

    //     case MODBUS_CMD_ZERO_LEARN_START:
    //         if (zero_learn_manual_start(&g_parameters, &g_algo_state))
    //         {
    //             g_parameters.is_saved = 1U;
    //             if (SaveParameters(&g_parameters) != E2PROM_OK)
    //             {
    //                 g_parameters.is_saved = 0U;
    //                 log_e("manual zero learn saved in RAM only");
    //             }
    //             else
    //             {
    //                 log_i("manual zero learn finished, offset=%.6f m/s",
    //                       g_parameters.zero_offset_speed);
    //             }
    //             update_holding_registers_from_parameters();
    //             update_input_registers();
    //         }
    //         else
    //         {
    //             log_e("manual zero learn rejected, check SQ/window/current flow");
    //         }
    //         break;

    //     case MODBUS_CMD_SAVE_PARAMETERS:
    //         g_parameters.is_saved = 1U;
    //         if (SaveParameters(&g_parameters) != E2PROM_OK)
    //         {
    //             g_parameters.is_saved = 0U;
    //             log_e("save parameters failed");
    //         }
    //         update_holding_registers_from_parameters();
    //         update_input_registers();
    //         break;

    //     case MODBUS_CMD_LOAD_DEFAULT_PARAMETERS:
    //         parameter_reset_to_default();
    //         g_parameters.is_saved = 1U;
    //         if (SaveParameters(&g_parameters) != E2PROM_OK)
    //         {
    //             g_parameters.is_saved = 0U;
    //             log_e("load default parameters saved in RAM only");
    //         }
    //         update_holding_registers_from_parameters();
    //         update_input_registers();
    //         break;

    //     case MODBUS_CMD_CLEAR_ALARM:
    //         g_alarm = ALARM_OK;
    //         update_input_registers();
    //         break;

    //     case MODBUS_CMD_SOFT_RESET:
    //         g_parameters.is_saved = 1U;
    //         if (SaveParameters(&g_parameters) != E2PROM_OK)
    //         {
    //             g_parameters.is_saved = 0U;
    //             log_e("save parameters before reset failed");
    //         }
    //         NVIC_SystemReset();
    //         break;

    //     default:
    //         break;
    //     }
    // }
}
