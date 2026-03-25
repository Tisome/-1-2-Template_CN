#ifndef DOES_IT_WORK_H
#define DOES_IT_WORK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t task_modbus_handler;

void task_clock(void *p);
void task_elog(void *p);
void task_e2prom(void *p);
void task_modbus(void *p);

void does_it_work(void);

#ifdef __cplusplus
}
#endif

#endif /* DOES_IT_WORK_H */