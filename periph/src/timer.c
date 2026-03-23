#include "gd32f30x.h"

#include "timer.h"

/* =========================
 * Modbus 定时器初始化
 * TIMER2: 1us 计数，4ms 超时
 * ========================= */
void modbus_timer_init(void)
{
    timer_parameter_struct timer_initpara;

    timer_deinit(MODBUS_TIMER);

    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler = MODBUS_TIMER_PSC; /* 120MHz -> 1MHz */
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_initpara.period = MODBUS_TIMER_PERIOD; /* 4500us */
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0U;

    timer_init(MODBUS_TIMER, &timer_initpara);

    /* 允许更新中断 */
    timer_interrupt_enable(MODBUS_TIMER, TIMER_INT_UP);

    /* 中断优先级建议不高于 USART */
    nvic_irq_enable(MODBUS_TIMER_IRQn, 8U, 0U);

    /* 默认先停着，收到字节时再启动 */
    timer_disable(MODBUS_TIMER);
    timer_counter_value_config(MODBUS_TIMER, 0U);
    timer_interrupt_flag_clear(MODBUS_TIMER, TIMER_INT_FLAG_UP);
}

/* =========================
 * 重启帧间隔定时器
 * 立即返回，不阻塞
 * ========================= */
void modbus_timer_restart(void)
{
    timer_disable(MODBUS_TIMER);
    timer_counter_value_config(MODBUS_TIMER, 0U);
    timer_interrupt_flag_clear(MODBUS_TIMER, TIMER_INT_FLAG_UP);
    timer_enable(MODBUS_TIMER);
}

/* =========================
 * 停止帧间隔定时器
 * ========================= */
void modbus_timer_stop(void)
{
    timer_disable(MODBUS_TIMER);
    timer_counter_value_config(MODBUS_TIMER, 0U);
    timer_interrupt_flag_clear(MODBUS_TIMER, TIMER_INT_FLAG_UP);
}