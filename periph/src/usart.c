#include "usart.h"
#include "gpio.h"

#include <string.h>

#include "circular_buffer.h"

#include "FreeRTOS.h"
#include "task.h"

#include "elog.h"

uint16_t usart0_dma_get_pos(void)
{
    uint16_t pos;

    pos = MODBUS_RX_BUF_SIZE - dma_transfer_number_get(DMA0, DMA_CH4);
    if (pos >= MODBUS_RX_BUF_SIZE)
    {
        log_e("pos >= MODBUS_RX_BUF_SIZE");
        pos = 0;
    }
    return pos;
}

uint8_t usart0_read_byte(void)
{
#if CCT6
    return (uint8_t)usart_data_receive(USART0);
#elif RCT6
    return (uint8_t)usart_data_receive(USART0);
#else
    return (uint8_t)usart_data_receive(USART0);
#endif
}

void usart0_write_byte(uint8_t byte)
{
#if CCT6
    usart_data_transmit(USART0, byte);
#elif RCT6
    usart_data_transmit(USART0, byte);
#else
    usart_data_transmit(USART0, byte);
#endif
}

static void usart0_tx_begin(void)
{
#if CCT6
    /* RS485 board: DE = 1 (TX mode) */
    gpio_bit_set(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_PIN);
#elif RCT6
    /* USART board: no direction control */
#else
    /* default: no direction control */
#endif
}

static void usart0_tx_end(void)
{
#if CCT6
    /* RS485 board: DE = 0 (RX mode) */
    gpio_bit_reset(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_PIN);
#elif RCT6
    /* USART board: no direction control */
#else
    /* default: no direction control */
#endif
}

/* ======================
 * USART0 init with DMA
 * ====================== */
void usart0_dma_modbus_init(uint8_t *modbus_rx_dma_buf, uint16_t buf_size)
{
    dma_parameter_struct dma_init_struct;

    usart_deinit(USART0);
    usart_baudrate_set(USART0, 9600);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

    usart_interrupt_enable(USART0, USART_INT_IDLE);
    nvic_irq_enable(USART0_IRQn, 7, 0);

    dma_deinit(DMA0, DMA_CH4);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)modbus_rx_dma_buf;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = buf_size;
    dma_init_struct.periph_addr = (uint32_t)&USART_DATA(USART0);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);

    dma_circulation_enable(DMA0, DMA_CH4);
    dma_memory_to_memory_disable(DMA0, DMA_CH4);

    usart_dma_receive_config(USART0, USART_DENR_ENABLE);

    dma_channel_enable(DMA0, DMA_CH4);
    usart_enable(USART0);
}

/* =========================
 * Modbus bytes send
 * ========================= */
void usart0_send_modbus_bytes(const uint8_t *buf, uint32_t len)
{
    uint32_t i;

    if ((buf == NULL) || (len == 0U))
    {
        return;
    }

    usart0_tx_begin();

    for (i = 0U; i < len; i++)
    {
        while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
        {
        }
        usart0_write_byte(buf[i]);
    }

    while (RESET == usart_flag_get(USART0, USART_FLAG_TC))
    {
    }

    usart0_tx_end();
}