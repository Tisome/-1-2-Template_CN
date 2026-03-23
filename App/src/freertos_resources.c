/************************* freertos header file *************************/
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "data.h"

SemaphoreHandle_t xSem_key_Filter = NULL;  // 按键滤波信号量句柄
SemaphoreHandle_t xSem_FPGA_INT = NULL;    // FPGA中断信号量句柄
SemaphoreHandle_t xSem_SPI_Rx_Done = NULL; // SPI数据已接收信号量句柄
QueueHandle_t xQueue_Rx_Index_Buf = NULL;  // 接收数据队列句柄
QueueHandle_t xQueue_AlgoOut = NULL;       // 算法输出队列句柄

void freertos_resources_init(void)
{
    vSemaphoreInit();
    vQueueInit();
}

static void vSemaphoreInit(void)
{
    // 创建按键滤波信号量
    xSem_key_Filter = xSemaphoreCreateBinary();
    configASSERT(xSem_key_Filter != NULL);

    // 创建FPGA中断信号量
    xSem_FPGA_INT = xSemaphoreCreateBinary();
    configASSERT(xSem_FPGA_INT != NULL);

    // 创建SPI数据已接收信号量
    xSem_SPI_Rx_Done = xSemaphoreCreateBinary();
    configASSERT(xSem_SPI_Rx_Done != NULL);
}

static void vQueueInit(void)
{
    // 创建接收数据队列
    xQueue_Rx_Index_Buf = xQueueCreate(1, sizeof(rufx_raw_packet_t));
    configASSERT(xQueue_Rx_Index_Buf != NULL);

    // 创建算法输出队列
    xQueue_AlgoOut = xQueueCreate(1, sizeof(Pipe_algo_out_data_t));
    configASSERT(xQueue_AlgoOut != NULL);
}