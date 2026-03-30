#ifndef DOES_IT_WORK_H
#define DOES_IT_WORK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t task_modbus_handler;
extern TaskHandle_t task_spi_rx_handler;

void task_clock(void *p);
void task_elog(void *p);
void task_e2prom(void *p);
void task_modbus(void *p);
void task_spi_rx(void *p);
void task_key_test(void *p);

TaskHandle_t get_spi_rx_task_handle(void);

TaskHandle_t get_modbus_task_handle(void);

void does_it_work(void);

#ifdef __cplusplus
}
#endif

#endif /* DOES_IT_WORK_H */
