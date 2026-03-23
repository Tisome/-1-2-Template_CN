#ifndef __freertos_resources_h
#define __freertos_resources_h

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

extern SemaphoreHandle_t xSem_key_Filter;  // 按键滤波信号量句柄
extern SemaphoreHandle_t xSem_FPGA_INT;    // FPGA中断信号量句柄
extern SemaphoreHandle_t xSem_SPI_Rx_Done; // SPI数据已接收信号量句柄
extern QueueHandle_t xQueue_Rx_Index_Buf;  // 接收数据队列句柄
extern QueueHandle_t xQueue_AlgoOut;       // 算法输出队列句柄

void freertos_resources_init(void);

#endif