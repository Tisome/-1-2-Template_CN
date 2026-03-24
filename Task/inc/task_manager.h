#ifndef __TASK_MANAGER_H
#define __TASK_MANAGER_H

/************************* freertos header file *************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

void do_create_start_task(void);

void do_create_display_task(void);
TaskHandle_t get_display_task_handle(void);

void do_create_algorithm_task(void);
TaskHandle_t get_algorithm_task_handle(void);

void do_create_bsp_key_task(void);
TaskHandle_t get_bsp_key_task_handle(void);

void do_create_spi_rx_task(void);
TaskHandle_t get_spi_rx_task_handle(void);

void do_create_test_task(void);
TaskHandle_t get_test_task_handle(void);

void do_create_bsp_e2prom_task(void);
TaskHandle_t get_bsp_e2prom_task_handle(void);

void do_create_bsp_uart_task(void);
TaskHandle_t get_bsp_uart_task_handle(void);

void do_create_watchdog_task(void);
TaskHandle_t get_watchdog_task_handle(void);

void do_create_modbus_parse_task(void);
TaskHandle_t get_modbus_parse_task_handle(void);

void do_create_modbus_execute_task(void);
TaskHandle_t get_modbus_execute_task_handle(void);

#endif
