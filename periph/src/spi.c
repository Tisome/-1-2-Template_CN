#include "spi.h"

#include "gd32f30x.h"

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"

#include <stddef.h>

/* FPGA-SPI init */
void spi1_init()
{
    spi_parameter_struct spi_init_struct;

    spi_i2s_deinit(FPGA_SPI);

    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE; /* Mode 0 */
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_64;
    spi_init_struct.endian = SPI_ENDIAN_MSB;

    spi_init(FPGA_SPI, &spi_init_struct);

    spi_enable(FPGA_SPI);
}

/**
 * @brief 获取当前系统时间(ms)
 */
static uint32_t spi_get_tick_ms(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/**
 * @brief 根据 deadline 计算剩余超时时间
 * @param deadline_ms : 截止时间点
 * @param remain_ms   : 输出，剩余毫秒数
 * @retval SPI_STATUS_OK / SPI_STATUS_TIMEOUT
 */
static spi_status_t spi_get_remaining_timeout(uint32_t deadline_ms,
                                              uint32_t *remain_ms)
{
    uint32_t now = spi_get_tick_ms();

    if ((remain_ms == NULL))
    {
        return SPI_STATUS_ERROR_PARAMETER;
    }

    if (now >= deadline_ms)
    {
        *remain_ms = 0U;
        return SPI_STATUS_TIMEOUT;
    }

    *remain_ms = deadline_ms - now;
    return SPI_STATUS_OK;
}

/**
 * @brief 等待 SPI 某个标志位变成指定状态，带超时
 * @param spi_periph : SPI外设，例如 SPI1
 * @param flag       : SPI_FLAG_TBE / SPI_FLAG_RBNE / SPI_FLAG_TRANS
 * @param status     : SET / RESET
 * @param timeout_ms : 超时时间(ms)
 * @retval SPI_STATUS_OK / SPI_STATUS_TIMEOUT
 */
static spi_status_t spi_wait_flag(uint32_t spi_periph,
                                  uint32_t flag,
                                  FlagStatus status,
                                  uint32_t timeout_ms)
{
    TickType_t start_tick;
    TickType_t timeout_tick;

    start_tick = xTaskGetTickCount();
    timeout_tick = pdMS_TO_TICKS(timeout_ms);

    while (spi_i2s_flag_get(spi_periph, flag) != status)
    {
        if ((xTaskGetTickCount() - start_tick) >= timeout_tick)
        {
            return SPI_STATUS_TIMEOUT;
        }
    }

    return SPI_STATUS_OK;
}

/**
 * @brief SPI主机读取一包数据（阻塞方式，带超时）
 * @param spi_periph : SPI外设，例如 SPI1
 * @param rx_buf     : 接收缓冲区
 * @param len        : 接收长度（字节）
 * @param timeout_ms : 整包读取总超时时间（毫秒）
 * @retval SPI_STATUS_OK              成功
 * @retval SPI_STATUS_TIMEOUT         超时
 * @retval SPI_STATUS_ERROR_PARAMETER 参数错误
 *
 * @note
 * 1. SPI主机接收数据时，必须主动发送 dummy 数据产生时钟
 * 2. 本函数按“整包总超时”控制，不是每个字节单独重新计时
 * 3. 如果中途某一步卡住，会在 timeout_ms 内退出，不会死等
 */
spi_status_t spi_master_read_packet_timeout(uint32_t spi_periph,
                                            uint8_t *rx_buf,
                                            uint16_t len,
                                            uint32_t timeout_ms)
{
    uint16_t i;
    spi_status_t ret;
    uint32_t deadline_ms;
    uint32_t remain_ms;

    if ((rx_buf == NULL) || (len == 0U))
    {
        return SPI_STATUS_ERROR_PARAMETER;
    }

    if (timeout_ms == 0U)
    {
        timeout_ms = 10U;
    }

    deadline_ms = spi_get_tick_ms() + timeout_ms;

    for (i = 0; i < len; i++)
    {
        /* 等待发送缓冲区空(TBE=SET)，可以写入 dummy 数据 */
        ret = spi_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != SPI_STATUS_OK)
        {
            return ret;
        }

        ret = spi_wait_flag(spi_periph, SPI_FLAG_TBE, SET, remain_ms);
        if (ret != SPI_STATUS_OK)
        {
            return ret;
        }

        /* 主机发送 dummy 字节，产生时钟 */
        spi_i2s_data_transmit(spi_periph, 0x00U);

        /* 等待接收缓冲区非空(RBNE=SET)，说明收到1字节 */
        ret = spi_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != SPI_STATUS_OK)
        {
            return ret;
        }

        ret = spi_wait_flag(spi_periph, SPI_FLAG_RBNE, SET, remain_ms);
        if (ret != SPI_STATUS_OK)
        {
            return ret;
        }

        /* 读取收到的数据 */
        rx_buf[i] = (uint8_t)spi_i2s_data_receive(spi_periph);
    }

    /* 等待 SPI 总线空闲(TRANS=RESET)，确保最后一个字节真正传输完成 */
    ret = spi_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != SPI_STATUS_OK)
    {
        return ret;
    }

    ret = spi_wait_flag(spi_periph, SPI_FLAG_TRANS, RESET, remain_ms);
    if (ret != SPI_STATUS_OK)
    {
        return ret;
    }

    return SPI_STATUS_OK;
}