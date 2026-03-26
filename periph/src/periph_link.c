#include "periph_link.h"
#include "circular_buffer.h"
#include "usart.h"

#include "elog.h"

extern circular_buf_t *g_modbus_rx_cb;

static void modbus_rx_buf_init(void)
{
    g_modbus_rx_cb = create_empty_circular_buffer();
    if (g_modbus_rx_cb == NULL)
    {
        log_e("g_modbus_rx_cb init failed");
        return;
    }
}

void hardware_periph_init(void)
{
    SystemInit();

    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    rcu_config();
    gpio_config();

    modbus_rx_buf_init();

    usart0_dma_modbus_init(g_modbus_rx_cb->buffer, MODBUS_RX_BUF_SIZE);
}