/************************* freertos header file *************************/
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "data.h"

QueueHandle_t xQueue_Rx_Index_Buf = NULL;  // 接收数据队列句柄
QueueHandle_t xQueue_AlgoOut = NULL;       // 算法输出队列句柄


static void vQueueInit(void)
{
    // 创建接收数据队列
    xQueue_Rx_Index_Buf = xQueueCreate(1, sizeof(rufx_raw_packet_t));
    configASSERT(xQueue_Rx_Index_Buf != NULL);

    // 创建算法输出队列
    xQueue_AlgoOut = xQueueCreate(1, sizeof(Pipe_algo_out_data_t));
    configASSERT(xQueue_AlgoOut != NULL);
}

void freertos_resources_init(void)
{
    vQueueInit();
}
