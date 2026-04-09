#include "periph_link.h"
#include "app_config.h"
#include "circular_buffer.h"
#include "spi.h"
#include "usart.h"

#include "elog.h"

extern circular_buf_t *g_modbus_rx_cb;

void hardware_periph_init(void)
{
    SystemInit();

    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    rcu_config();

    gpio_config();

#if APP_ENABLE_MEASURE_FPGA_SPI
    spi1_init();
#endif

#if APP_ENABLE_MODBUS_RUNTIME
    g_modbus_rx_cb = modbus_buf_init();
    if (g_modbus_rx_cb != NULL)
    {
        usart0_dma_modbus_init(g_modbus_rx_cb->buffer, MODBUS_RX_BUF_SIZE);
    }
    else
    {
        log_e("modbus_buf_init failed");
    }
#else
    g_modbus_rx_cb = NULL;
#endif
}
