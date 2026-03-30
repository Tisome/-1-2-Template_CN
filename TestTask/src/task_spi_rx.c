#include "elog.h"
#include "spi.h"
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"

#define FPGA_PKT_LEN 24U

extern TaskHandle_t task_spi_rx_handle;

void task_spi_rx(void *p)
{
    uint8_t rx_buf[FPGA_PKT_LEN];
    spi_status_t ret;
    uint16_t i;

    (void)p;

    /* 默认拉高片选 */
    gpio_bit_set(FPGA_SPI_NSS_GPIO_PORT, FPGA_SPI_NSS_GPIO_PIN);

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        log_i("start spi rx");

        memset(rx_buf, 0, sizeof(rx_buf));

        ret = fpga_spi_read_packet(rx_buf, sizeof(rx_buf), 20U);
        if (ret != SPI_STATUS_OK)
        {
            log_e("spi read failed, ret=%d", ret);
            continue;
        }

        log_i("spi rx done");
        for (i = 0; i < FPGA_PKT_LEN; i++)
        {
            log_i("rx_buf[%d] = 0x%02X", i, rx_buf[i]);
        }

        /* 后续可在这里做包头、CRC、解析 */
    }
}