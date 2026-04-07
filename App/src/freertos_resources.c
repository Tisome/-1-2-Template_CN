/*
 * FreeRTOS 共享资源初始化文件。
 * 本文件负责创建跨任务共享的消息队列，供数据采集、算法和显示等子系统通信。
 */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "data.h"

QueueHandle_t xQueue_Rx_Index_Buf = NULL;  // 接收数据队列句柄
QueueHandle_t xQueue_AlgoOut = NULL;       // 算法输出队列句柄


/* 创建系统运行依赖的公共队列。 */
static void vQueueInit(void)
{
    // 创建接收数据队列
    xQueue_Rx_Index_Buf = xQueueCreate(1, sizeof(rufx_raw_packet_t));
    configASSERT(xQueue_Rx_Index_Buf != NULL);

    // 创建算法输出队列
    xQueue_AlgoOut = xQueueCreate(1, sizeof(Pipe_algo_out_data_t));
    configASSERT(xQueue_AlgoOut != NULL);
}

/* FreeRTOS 资源初始化总入口，当前主要负责队列创建。 */
void freertos_resources_init(void)
{
    vQueueInit();
}
