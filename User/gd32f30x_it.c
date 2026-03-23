/*!
    \file    gd32f30x_it.c
    \brief   interrupt service routines

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f30x_it.h"

#include "FreeRTOS.h"

#include "gpio.h"
#include "timer.h"
#include "usart.h"

extern void xPortSysTickHandler(void);

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
    ;
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/

void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
// void SVC_Handler(void)
// {
// }

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
// void PendSV_Handler(void)
// {
// }

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
// void SysTick_Handler(void)
// {
//     xPortSysTickHandler();
// }

// 以下为中断服务函数，用户可根据需要，在对应函数内添加内容

/*-------------------------------------------------------
 * 按键 中断
 *------------------------------------------------------*/

/* EXTI0(KEY1)  中断服务函数 */
void EXTI0_IRQHandler(void)
{
    if (exti_interrupt_flag_get(KEY1_EXTI) != RESET)
    {
        exti_interrupt_flag_clear(KEY1_EXTI);
        GPIO_EXTI_IRQHandler(KEY1_GPIO_PIN);
    }
}

/* EXTI5_9(KEY2)  中断服务函数 */
void EXTI5_9_IRQHandler(void)
{
    if (exti_interrupt_flag_get(KEY2_EXTI) != RESET)
    {
        exti_interrupt_flag_clear(KEY2_EXTI);
        GPIO_EXTI_IRQHandler(KEY2_GPIO_PIN);
    }
}

/* EXTI1(KEY3)  中断服务函数 */
void EXTI1_IRQHandler(void)
{
    if (exti_interrupt_flag_get(KEY3_EXTI) != RESET)
    {
        exti_interrupt_flag_clear(KEY3_EXTI);
        GPIO_EXTI_IRQHandler(KEY3_GPIO_PIN);
    }
}

/* EXTI2(KEY4)  中断服务函数 */
void EXTI2_IRQHandler(void)
{
    if (exti_interrupt_flag_get(KEY4_EXTI) != RESET)
    {
        exti_interrupt_flag_clear(KEY4_EXTI);
        GPIO_EXTI_IRQHandler(KEY4_GPIO_PIN);
    }
}

/* EXTI10_15(FPGA_INT) 中断服务函数 */
void EXTI10_15_IRQHandler(void)
{
    if (exti_interrupt_flag_get(FPGA_INT_EXTI) != RESET)
    {
        exti_interrupt_flag_clear(FPGA_INT_EXTI);
        GPIO_EXTI_IRQHandler(FPGA_INT_GPIO_PIN);
    }
}

void fpga_int_gpio_exti_handler(uint32_t GPIO_PIN_x)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (GPIO_PIN_x == FPGA_INT_GPIO_PIN)
    {
        xSemaphoreGiveFromISR(xSem_FPGA_INT, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void GPIO_EXTI_IRQHandler(uint32_t GPIO_PIN_x)
{
    switch (GPIO_PIN_x)
    {
    case (KEY1_GPIO_PIN):
    case (KEY2_GPIO_PIN):
    case (KEY3_GPIO_PIN):
    case (KEY4_GPIO_PIN):
        key_gpio_exti_handler(GPIO_PIN_x);
        break;
    case (FPGA_INT_GPIO_PIN):
        fpga_int_gpio_exti_handler(GPIO_PIN_x);
    }
}

/* =========================
 * USART0 中断
 * 每收到1字节：
 * 1. 取数据
 * 2. 写缓冲区
 * 3. 重启3.5T定时器
 * ========================= */
void USART0_IRQHandler(void)
{
    uint8_t data;

    /* 接收缓冲非空 */
    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE))
    {
        data = (uint8_t)usart_data_receive(USART0);

        /*
         * 如果上一帧还没被任务取走，这里简单丢弃新数据
         * 对 Modbus 从机通常够用，因为一般是一问一答
         */
        if (0U == s_modbus_frame_ready)
        {
            if (s_modbus_rx_len < MODBUS_RX_BUF_SIZE)
            {
                s_modbus_rx_buf[s_modbus_rx_len++] = data;
                modbus_timer_restart();
            }
            else
            {
                /* 缓冲区溢出 */
                s_modbus_rx_overflow = 1U;
            }
        }
    }

    /*
     * 错误处理
     * 某些库里还可以分别处理:
     * USART_FLAG_ORERR / NERR / FERR
     */
    if (RESET != usart_flag_get(USART0, USART_FLAG_ORERR))
    {
        volatile uint32_t stat;
        volatile uint32_t data_dummy;

        stat = USART_STAT0(USART0);
        data_dummy = USART_DATA(USART0);
        (void)stat;
        (void)data_dummy;

        modbus_timer_stop();
        s_modbus_rx_len = 0U;
        s_modbus_frame_ready = 0U;
        s_modbus_rx_overflow = 0U;
    }
}

/* =========================
 * TIMER2 中断
 * 到时表示：超过3.5T没有新字节，判定一帧完成
 * ========================= */
void TIMER2_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (SET == timer_interrupt_flag_get(MODBUS_TIMER, TIMER_INT_FLAG_UP))
    {
        timer_interrupt_flag_clear(MODBUS_TIMER, TIMER_INT_FLAG_UP);

        /* 单次触发，进来后先停掉 */
        modbus_timer_stop();

        if ((s_modbus_rx_len > 0U) && (0U == s_modbus_rx_overflow))
        {
            s_modbus_frame_ready = 1U;

            if (g_modbus_task_handle != NULL)
            {
                vTaskNotifyGiveFromISR(g_modbus_task_handle, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        else
        {
            /* 出错或空帧则清掉 */
            s_modbus_rx_len = 0U;
            s_modbus_frame_ready = 0U;
            s_modbus_rx_overflow = 0U;
        }
    }
}

// /* EXTI0 中断服务函数 */
// void EXTI0_IRQHandler(void)
// {
//     ;
// }

// /* EXTI1 中断服务函数 */
// void EXTI1_IRQHandler(void)
// {
//     ;
// }

// /* EXTI2 中断服务函数 */
// void EXTI2_IRQHandler(void)
// {
//     ;
// }
// /* EXTI3 中断服务函数 */
// void EXTI3_IRQHandler(void)
// {
//     ;
// }
// /* EXTI4 中断服务函数 */
// void EXTI4_IRQHandler(void)
// {
//     ;
// }
// /* EXTI5_9 中断服务函数 */
// void EXTI5_9_IRQHandler(void)
// {
//     ;
// }
// /* EXTI10_15 中断服务函数 */
// void EXTI10_15_IRQHandler(void)
// {
//     ;
// }

// /* TIMER0_Channel 中断服务函数 */
// void TIMER0_Channel_IRQHandler(void)
// {
//     ;
// }

// /* TIMER1 中断服务函数 */
// void TIMER1_IRQHandler(void)
// {
//     ;
// }

// /* TIMER2 中断服务函数 */
// void TIMER2_IRQHandler(void)
// {
//     ;
// }

// /* TIMER3 中断服务函数 */
// void TIMER3_IRQHandler(void)
// {
//     ;
// }

// /* TIMER4 中断服务函数 */
// void TIMER4_IRQHandler(void)
// {
//     ;
// }

// /* TIMER5 中断服务函数 */
// void TIMER5_IRQHandler(void)
// {
//     ;
// }

// /* TIMER6 中断服务函数 */
// void TIMER6_IRQHandler(void)
// {
//     ;
// }

// /* TIMER7 中断服务函数 */
// void TIMER7_Channel_IRQHandler(void)
// {
//     ;
// }

// /* I2C0_EV 中断服务函数 */
// void I2C0_EV_IRQHandler(void)
// {
//     ;
// }

// /* I2C1_EV 中断服务函数 */
// void I2C1_EV_IRQHandler(void)
// {
//     ;
// }

// /* SPI0 中断服务函数 */
// void SPI0_IRQHandler(void)
// {
//     ;
// }

// /* SPI1 中断服务函数 */
// void SPI1_IRQHandler(void)
// {
//     ;
// }

// /* SPI2 中断服务函数 */
// void SPI2_IRQHandler(void)
// {
//     ;
// }

// /* USART0 中断服务函数 */
// void USART0_IRQHandler(void)
// {
//     ;
// }

// /* USART1 中断服务函数 */
// void USART1_IRQHandler(void)
// {
//     ;
// }

// /* USART2 中断服务函数 */
// void USART2_IRQHandler(void)
// {
//     ;
// }

// /* UART3 中断服务函数 */
// void UART3_IRQHandler(void)
// {
//     ;
// }

// /* USART4 中断服务函数 */
// void UART4_IRQHandler(void)
// {
//     ;
// }
