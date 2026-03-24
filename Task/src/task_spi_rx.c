/**
 * 功能说明：
 * 1) FPGA 在有一帧数据准备好后，通过 FPGA_INT 引脚触发中断通知 MCU。
 *    在 FPGA_INT 的 EXTI 中断服务函数（ISR）中会释放 xSem_FPGA_INT 信号量。
 *
 * 2) 本任务（vSPI_Rx_task）一直阻塞等待 xSem_FPGA_INT：
 *    - 一旦拿到信号量，说明 FPGA 提示“可以开始读数据”
 *    - 拉低片选 CS（NSS），选通 FPGA
 *    - 调用 HAL_SPI_Receive_DMA() 启动 DMA 接收固定长度的数据帧
 *    - 阻塞等待 DMA 完成信号量 xSem_SPI_Rx_Done
 *      （通常在 HAL_SPI_RxCpltCallback() 回调里释放该信号量）
 *    - DMA 接收完成后拉高 CS，结束本次 SPI 帧读
 *
 * 3) 将 DMA 接收缓冲区中的字节流拷贝到 rufx_raw_packet_t 结构体 packet 中，
 *    然后通过队列 xQueue_Rx_Index_Buf 发送给算法任务（algorithmTask）。
 *
 * 关键点说明：
 * - Rx_Index_Buf 是 SPI DMA 的接收缓冲区，DMA 会直接把 SPI 接收到的数据写入该数组。
 * - xQueueSend() 发送的是 packet 结构体“内容的拷贝”，不是指针。
 *   前提是创建队列时 item_size = sizeof(rufx_raw_packet_t)。
 * - 本版本按 8-bit 数据帧接收，RUF_X_PACKET_SIZE_BYTES 表示一帧数据的字节数。
 */

#include "gd32f30x.h"

/************************* freertos header file *************************/
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/************************* perph header file *************************/
#include "gpio.h"
#include "spi.h"

/************************* app header file *************************/
#include "data.h"
#include "elog.h"
#include "freertos_resources.h"
#include "task_manager.h"

/************************* std header file *************************/
#include <string.h>

void task_spi_rx(void *pvParameter)
{
    rufx_raw_packet_t packet;
    spi_status_t ret;
    static uint8_t seq_cnt = 0;

    (void)pvParameter;

    /* 默认释放片选 */
    FPGA_SPI_CS_DISABLE();

    for (;;)
    {
        /* 1. 等待 FPGA_INT 中断通知 */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* 2. 拉低 CS，开始读取 FPGA 数据 */
        FPGA_SPI_CS_ENABLE();

        /* 3. 读取一整包原始数据到 packet.bytes */
        ret = spi_master_read_packet_timeout(FPGA_SPI,
                                             packet.bytes,
                                             RUF_X_PACKET_SIZE_BYTES,
                                             3U);

        /* 4. 拉高 CS，结束本次 SPI 通信 */
        FPGA_SPI_CS_DISABLE();

        if (ret == SPI_STATUS_OK)
        {
            /* 5. 给包加一个序号，方便后续调试丢包/乱序 */
            packet.seq = seq_cnt++;

            /* 6. 把原始包送到队列 */
            xQueueOverwrite(xQueue_Rx_Index_Buf, &packet);
        }
        else
        {
            log_e("receive FPGA spi error");
        }
    }
}

static TaskHandle_t task_spi_rx_handle = NULL; /* 创建任务句柄 */

TaskHandle_t get_spi_rx_task_handle(void)
{
    return task_spi_rx_handle;
}

void do_create_spi_rx_task(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */
    /* 创建AppTaskCreate任务 */
    xReturn = xTaskCreate(task_spi_rx,
                          "task_spi_rx",
                          128,
                          NULL,
                          9,
                          &task_spi_rx_handle);
    if (xReturn != pdPASS)
    {
        log_e("Failed to create task_spi_rx");
    }
}