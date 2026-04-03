#ifndef __freertos_resources_h
#define __freertos_resources_h

#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t xQueue_Rx_Index_Buf;  // 接收数据队列句柄
extern QueueHandle_t xQueue_AlgoOut;       // 算法输出队列句柄

void freertos_resources_init(void);

#endif
