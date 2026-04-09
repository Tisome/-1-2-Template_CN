#include "elog.h"
#include "algorithm_packet.h"
#include "data.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"

#include <string.h>

extern TaskHandle_t task_spi_rx_handle;

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

void task_spi_rx(void *p)
{
    uint8_t rx_buf[RUF_X_PACKET_SIZE_BYTES];
    rufx_raw_packet_t raw = {0};
    rufx_packet_t pkt = {0};
    spi_status_t ret;
    uint32_t packet_count = 0U;
    uint16_t i;

    (void)p;

    gpio_bit_set(FPGA_SPI_NSS_GPIO_PORT, FPGA_SPI_NSS_GPIO_PIN);
    fpga_test_start_allow();
    vTaskDelay(pdMS_TO_TICKS(100));

    while (1)
    {
        while (ulTaskNotifyTake(pdTRUE, 0U) != 0U)
        {
        }

        if (fpga_test_int_is_asserted() == pdTRUE)
        {
            log_i("fpga_int already asserted, skip wait");
        }
        else if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)) == 0U)
        {
            log_e("wait fpga_int timeout");
            continue;
        }

        fpga_test_start_holdoff();

        memset(rx_buf, 0, sizeof(rx_buf));

        ret = fpga_spi_read_packet(rx_buf, sizeof(rx_buf), 20U);
        if (ret != SPI_STATUS_OK)
        {
            log_e("spi read failed, ret=%d", ret);
            fpga_test_start_allow();
            continue;
        }

        fpga_test_start_allow();
        if (fpga_test_wait_int_clear(pdMS_TO_TICKS(20)) == pdFALSE)
        {
            log_e("wait fpga_int clear timeout");
        }
        packet_count++;

        if (packet_count <= 2U)
        {
            log_i("spi rx done, len=%u", (unsigned int)RUF_X_PACKET_SIZE_BYTES);
            for (i = 0; i < RUF_X_PACKET_SIZE_BYTES; i++)
            {
                log_i("rx_buf[%u] = 0x%02X", (unsigned int)i, rx_buf[i]);
            }
        }

        memcpy(raw.bytes, rx_buf, sizeof(rx_buf));
        raw.seq = 0U;
        rufx_unpack_packet(&raw, &pkt, NULL);

        if ((packet_count <= 8U) || ((packet_count % 16U) == 0U))
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
