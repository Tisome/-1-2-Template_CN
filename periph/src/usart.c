// #include "usart.h"
// #include "gpio.h"

// /* DMA 外设地址 */
// #define USART0_DATA_ADDRESS ((uint32_t)&USART_DATA(USART0))

// /* 内部变量：用于计算接收长度 */
// static uint32_t s_dma_buf_len = 0;
// static uint8_t *s_dma_buf = 0;

// /*-------------------------------------------------------
//  * USART0 + DMA 初始化
//  *------------------------------------------------------*/
// void usart0_dma_init(uint8_t *rxbuffer, uint32_t buflen)
// {
//     dma_parameter_struct dma_init_struct;

//     if ((rxbuffer == 0U) || (buflen == 0U))
//     {
//         return;
//     }

//     s_dma_buf = rxbuffer;
//     s_dma_buf_len = buflen;

//     /* USART config */
//     usart_deinit(USART0);

//     usart_baudrate_set(USART0, 9600U);
//     usart_parity_config(USART0, USART_PM_NONE);
//     usart_word_length_set(USART0, USART_WL_8BIT);
//     usart_stop_bit_set(USART0, USART_STB_1BIT);

//     usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
//     usart_receive_config(USART0, USART_RECEIVE_ENABLE);

//     /* IDLE interrupt */
//     usart_interrupt_enable(USART0, USART_INT_IDLE);
//     nvic_irq_enable(USART0_IRQn, 5U, 0U);

//     /* DMA config */
//     dma_deinit(DMA0, DMA_CH4);
//     dma_struct_para_init(&dma_init_struct);

//     dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
//     dma_init_struct.memory_addr = (uint32_t)rxbuffer;
//     dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
//     dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
//     dma_init_struct.number = buflen;
//     dma_init_struct.periph_addr = USART0_DATA_ADDRESS;
//     dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
//     dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
//     dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;

//     dma_init(DMA0, DMA_CH4, &dma_init_struct);

//     // circular buffer
//     dma_circulation_enable(DMA0, DMA_CH4);
//     dma_memory_to_memory_disable(DMA0, DMA_CH4);

//     /* DMA interrupt */
//     dma_interrupt_enable(DMA0, DMA_CH4, DMA_INT_HTF);
//     dma_interrupt_enable(DMA0, DMA_CH4, DMA_INT_FTF);
//     nvic_irq_enable(DMA0_Channel4_IRQn, 5U, 0U);

//     /* enable DMA */
//     usart_dma_receive_config(USART0, USART_DENR_ENABLE);
//     dma_channel_enable(DMA0, DMA_CH4);

//     /* enable USART */
//     usart_enable(USART0);
// }

// /*-------------------------------------------------------
//  * 普通发送（阻塞）
//  *------------------------------------------------------*/
// static void usart0_send_byte(uint8_t ch)
// {
//     usart_data_transmit(USART0, ch);

//     while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
//         ;
// }

// void usart0_send_bytes(uint8_t *buf, uint32_t len)
// {
//     uint32_t i;

//     if ((buf == 0U) || (len == 0U))
//     {
//         return;
//     }

//     for (i = 0U; i < len; i++)
//     {
//         usart0_send_byte(buf[i]);
//     }
// }

// /*-------------------------------------------------------
//  * Modbus / RS485 发送
//  *------------------------------------------------------*/
// void usart0_send_modbus_bytes(uint8_t *buf, uint32_t len)
// {
//     uint32_t i;

//     if ((buf == 0U) || (len == 0U))
//     {
//         return;
//     }

//     /* 切到发送模式 */
//     gpio_bit_set(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_PIN);

//     for (i = 0U; i < len; i++)
//     {
//         usart_data_transmit(USART0, buf[i]);
//         while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
//             ;
//     }

//     /* 等最后一位发完 */
//     while (usart_flag_get(USART0, USART_FLAG_TC) == RESET)
//         ;

//     /* 切回接收 */
//     gpio_bit_reset(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_PIN);
// }

// /*-------------------------------------------------------
//  * 获取当前 DMA 已接收长度
//  *------------------------------------------------------*/
// static uint32_t usart0_get_dma_rx_len(void)
// {
//     uint32_t remain;

//     remain = dma_transfer_number_get(DMA0, DMA_CH4);

//     return (s_dma_buf_len - remain);
// }

#include "usart.h"
#include "gpio.h"
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

/*
 * 你自己的 Modbus 任务句柄
 * 在别的 .c 文件里定义，例如:
 * TaskHandle_t g_modbus_task_handle = NULL;
 */
extern TaskHandle_t g_modbus_task_handle;

/* =========================
 * 全局变量
 * ========================= */
volatile uint8_t s_modbus_rx_buf[MODBUS_RX_BUF_SIZE];
volatile uint16_t s_modbus_rx_len = 0U;
volatile uint8_t s_modbus_frame_ready = 0U;
volatile uint8_t s_modbus_rx_overflow = 0U;

/* =========================
 * 内部函数
 * ========================= */
static void usart0_send_byte(uint8_t ch)
{
    while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
    {
    }

    usart_data_transmit(USART0, ch);
}

/* =========================
 * USART0 初始化（不用DMA）
 * ========================= */
void usart0_modbus_init(void)
{
    usart_deinit(USART0);

    usart_baudrate_set(USART0, MODBUS_BAUDRATE);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);

    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);

    /* 开接收中断 */
    usart_interrupt_enable(USART0, USART_INT_RBNE);

    /* 开错误中断（溢出/噪声/帧错） */
    usart_interrupt_enable(USART0, USART_INT_ERR);

    /* 中断优先级建议比 SPI DMA 低一点 */
    nvic_irq_enable(USART0_IRQn, 6U, 0U);

    usart_enable(USART0);

    /* 上电默认切回接收态 */
    gpio_bit_reset(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_PIN);

    modbus_rx_reset();
}

/* =========================
 * 清空接收状态
 * ========================= */
void modbus_rx_reset(void)
{
    __disable_irq();

    s_modbus_rx_len = 0U;
    s_modbus_frame_ready = 0U;
    s_modbus_rx_overflow = 0U;
    memset((void *)s_modbus_rx_buf, 0, sizeof(s_modbus_rx_buf));

    __enable_irq();
}

/* =========================
 * 查询是否收到完整帧
 * ========================= */
uint8_t modbus_frame_is_ready(void)
{
    return s_modbus_frame_ready;
}

/* =========================
 * 取出一帧
 * 取完后自动复位状态
 * ========================= */
uint16_t modbus_get_frame(uint8_t *buf, uint16_t max_len)
{
    uint16_t len;

    if ((buf == NULL) || (max_len == 0U))
    {
        return 0U;
    }

    if (0U == s_modbus_frame_ready)
    {
        return 0U;
    }

    __disable_irq();

    len = s_modbus_rx_len;
    if (len > max_len)
    {
        len = max_len;
    }

    memcpy(buf, (const void *)s_modbus_rx_buf, len);

    s_modbus_rx_len = 0U;
    s_modbus_frame_ready = 0U;
    s_modbus_rx_overflow = 0U;

    __enable_irq();

    return len;
}

/* =========================
 * 普通阻塞发送
 * ========================= */
void usart0_send_bytes(uint8_t *buf, uint32_t len)
{
    uint32_t i;

    if ((buf == NULL) || (len == 0U))
    {
        return;
    }

    for (i = 0U; i < len; i++)
    {
        usart0_send_byte(buf[i]);
    }

    while (RESET == usart_flag_get(USART0, USART_FLAG_TC))
    {
    }
}

/* =========================
 * RS485 / Modbus 发送
 * ========================= */
void usart0_send_modbus_bytes(uint8_t *buf, uint32_t len)
{
    uint32_t i;

    if ((buf == NULL) || (len == 0U))
    {
        return;
    }

    /* 切到发送模式（DE=1，RE根据你的电路决定） */
    gpio_bit_set(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_PIN);

    for (i = 0U; i < len; i++)
    {
        while (RESET == usart_flag_get(USART0, USART_FLAG_TBE))
        {
        }
        usart_data_transmit(USART0, buf[i]);
    }

    /* 等最后1位完全发完 */
    while (RESET == usart_flag_get(USART0, USART_FLAG_TC))
    {
    }

    /* 切回接收模式 */
    gpio_bit_reset(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_PIN);
}
