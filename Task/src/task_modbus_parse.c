#include "modbus_parse.h"
#include "task_manager.h"

#include "elog.h"

static TaskHandle_t task_modbus_parse_handle = NULL;

void do_create_modbus_parse_task(void)
{
    BaseType_t result = pdPASS;
    result = xTaskCreate(task_modbus_parse,
                         "task modbus parse",
                         512,
                         NULL,
                         2,
                         &task_modbus_parse_handle);
    if (result != pdPASS)
    {
        log_e("Failed to create Modbus parse task");
    }
}

TaskHandle_t get_modbus_parse_task_handle(void)
{
    return task_modbus_parse_handle;
}