#include "periph_link.h"
#include "circular_buffer.h"
#include "usart.h"

#include "lcd.h"
#include "lcd_init.h"

#include "elog.h"

extern circular_buf_t *g_modbus_rx_cb;

void hardware_periph_init(void)
{
    SystemInit();

    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    rcu_config();

    gpio_config();

    LCD_GPIO_Init();
    LCD_Init();
    Draw_Circle(100U, 100U, 10U, BLUE);

    g_modbus_rx_cb = modbus_buf_init();

    usart0_dma_modbus_init(g_modbus_rx_cb->buffer, MODBUS_RX_BUF_SIZE);
}