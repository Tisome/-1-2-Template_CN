#define LOG_TAG "spi_rx"
#define LOG_LVL ELOG_LVL_INFO

#include "algorithm_packet.h"
#include "app_config.h"
#include "data.h"
#include "elog.h"
#include "freertos_resources.h"
#include "gpio.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <string.h>

static void fpga_test_start_allow(void)
{
    gpio_bit_set(FPGA_START_GPIO_PORT, FPGA_START_GPIO_PIN);
}

static void fpga_test_start_holdoff(void)
{
    gpio_bit_reset(FPGA_START_GPIO_PORT, FPGA_START_GPIO_PIN);
}

static BaseType_t fpga_test_int_is_asserted(void)
{
    return (gpio_input_bit_get(FPGA_INT_GPIO_PORT, FPGA_INT_GPIO_PIN) == SET) ? pdTRUE : pdFALSE;
}

static BaseType_t fpga_test_wait_int_clear(TickType_t timeout_ticks)
{
    TickType_t start_tick = xTaskGetTickCount();

    while (fpga_test_int_is_asserted() == pdTRUE)
    {
        if ((xTaskGetTickCount() - start_tick) >= timeout_ticks)
        {
            return pdFALSE;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return pdTRUE;
}

static BaseType_t spi_rx_should_log_packet(uint32_t packet_count)
{
    if (packet_count <= APP_SPI_RX_VERBOSE_PACKETS)
    {
        return pdTRUE;
    }

    if ((APP_SPI_RX_PACKET_LOG_INTERVAL != 0U) &&
        ((packet_count % APP_SPI_RX_PACKET_LOG_INTERVAL) == 0U))
    {
        return pdTRUE;
    }

    return pdFALSE;
}

static BaseType_t spi_rx_should_log_asserted(uint32_t packet_count)
{
    if (packet_count < APP_SPI_RX_VERBOSE_PACKETS)
    {
        return pdTRUE;
    }

    if ((APP_SPI_RX_ASSERTED_LOG_INTERVAL != 0U) &&
        ((packet_count % APP_SPI_RX_ASSERTED_LOG_INTERVAL) == 0U))
    {
        return pdTRUE;
    }

    return pdFALSE;
}

void task_spi_rx(void *p)
{
    uint8_t rx_buf[RUF_X_PACKET_SIZE_BYTES];
    rufx_raw_packet_t raw = {0};
    rufx_packet_t pkt = {0};
    spi_status_t ret;
    uint32_t packet_count = 0U;
    uint32_t wait_timeout_count = 0U;
    uint16_t i;

    (void)p;

    gpio_bit_set(FPGA_SPI_NSS_GPIO_PORT, FPGA_SPI_NSS_GPIO_PIN);
    fpga_test_start_allow();
    vTaskDelay(pdMS_TO_TICKS(100));

    log_i("task_spi_rx start, source=FPGA SPI");

    while (1)
    {
        while (ulTaskNotifyTake(pdTRUE, 0U) != 0U)
        {
        }

        if (xQueue_Rx_Index_Buf == NULL)
        {
            log_e("xQueue_Rx_Index_Buf is NULL");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        if (fpga_test_int_is_asserted() == pdTRUE)
        {
            if (spi_rx_should_log_asserted(packet_count) == pdTRUE)
            {
                log_i("fpga_int already asserted, skip wait");
            }
        }
        else if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(APP_SPI_RX_WAIT_TIMEOUT_MS)) == 0U)
        {
            wait_timeout_count++;
            if ((wait_timeout_count <= APP_SPI_RX_VERBOSE_PACKETS) ||
                ((APP_SPI_RX_ASSERTED_LOG_INTERVAL != 0U) &&
                 ((wait_timeout_count % APP_SPI_RX_ASSERTED_LOG_INTERVAL) == 0U)))
            {
                log_e("wait fpga_int timeout");
            }
            continue;
        }

        fpga_test_start_holdoff();
        memset(rx_buf, 0, sizeof(rx_buf));

        ret = fpga_spi_read_packet(rx_buf, sizeof(rx_buf), APP_SPI_RX_READ_TIMEOUT_MS);
        fpga_test_start_allow();

        if (ret != SPI_STATUS_OK)
        {
            log_e("spi read failed, ret=%d", ret);
            continue;
        }

        if (fpga_test_wait_int_clear(pdMS_TO_TICKS(APP_SPI_RX_INT_CLEAR_TIMEOUT_MS)) == pdFALSE)
        {
            log_w("wait fpga_int clear timeout");
        }

        packet_count++;
        memcpy(raw.bytes, rx_buf, sizeof(rx_buf));
        raw.seq = (uint8_t)(packet_count & 0xFFU);
        (void)xQueueOverwrite(xQueue_Rx_Index_Buf, &raw);

        rufx_unpack_packet(&raw, &pkt, NULL);

        if (packet_count <= APP_SPI_RX_VERBOSE_PACKETS)
        {
            log_i("spi rx done, len=%u", (unsigned int)RUF_X_PACKET_SIZE_BYTES);
            for (i = 0; i < RUF_X_PACKET_SIZE_BYTES; i++)
            {
                log_i("rx_buf[%u] = 0x%02X", (unsigned int)i, rx_buf[i]);
            }
        }

        if (spi_rx_should_log_packet(packet_count) == pdTRUE)
        {
            log_i("spi pkt#%lu: idxA=%u idxB=%u y1=%lld y2=%lld y3=%lld",
                  (unsigned long)packet_count,
                  (unsigned int)pkt.idx_a,
                  (unsigned int)pkt.idx_b,
                  (long long)pkt.conv_y1,
                  (long long)pkt.conv_y2,
                  (long long)pkt.conv_y3);
        }
    }
}
