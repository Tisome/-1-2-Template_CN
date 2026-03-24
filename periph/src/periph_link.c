#include "periph_link.h"
#include "circular_buffer.h"
#include "usart.h"

#include "elog.h"

extern circular_buf_t *g_modbus_rx_cb;

void hardware_periph_init()
{
    rcu_config();
    gpio_config();

    // // 初始化usart0-dma写入后的数据地址
    // create_empty_circular_buffer(g_modbus_rx_cb);
    // usart0_dma_modbus_init(g_modbus_rx_cb->buffer, MODBUS_RX_BUF_SIZE);
}