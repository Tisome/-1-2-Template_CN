// #include "FreeRTOS.h"
// #include "task.h"

// #include "iic.h"

// #include <stddef.h>

// static iic_status_t iic_wait_flag_set(uint32_t i2c_periph,
//                                       i2c_flag_enum flag,
//                                       FlagStatus status,
//                                       uint32_t timeout)
// {
//     TickType_t start_tick = xTaskGetTickCount();
//     TickType_t timeout_tick = pdMS_TO_TICKS(timeout);

//     while (i2c_flag_get(i2c_periph, flag) != status)
//     {

//         if ((xTaskGetTickCount() - start_tick) >= timeout_tick)
//         {
//             return IIC_TIMEOUT;
//         }
//     }
//     return IIC_OK;
// }

// static void iic_deinit(uint32_t i2c_periph)
// {
//     switch (i2c_periph)
//     {
//     case I2C0:
//         /* reset I2C0 */
//         rcu_periph_reset_enable(RCU_I2C0RST);
//         rcu_periph_reset_disable(RCU_I2C0RST);
//         break;
//     case I2C1:
//         /* reset I2C1 */
//         rcu_periph_reset_enable(RCU_I2C1RST);
//         rcu_periph_reset_disable(RCU_I2C1RST);
//         break;
//     default:
//         break;
//     }
// }

// iic_status_t iic_init(uint32_t i2c_periph, uint32_t clock_speed)
// {
//     /* 1. 时钟使能 在rcu.c中完成*/

//     /* 2. GPIO 配置：I2C 必须 AF_OD 在gpio.c中完成*/

//     /* 3. I2C 复位 */
//     i2c_deinit(i2c_periph);

//     /* 4. I2C 参数配置 */
//     i2c_clock_config(i2c_periph, clock_speed, I2C_DTCY_2);
//     i2c_mode_addr_config(i2c_periph, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
//     i2c_enable(i2c_periph);
//     i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);

//     return IIC_OK;
// }

// static uint32_t iic_get_tick_ms(void)
// {
//     return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
// }

// static iic_status_t iic_get_remaining_timeout(uint32_t deadline_ms,
//                                               uint32_t *remain_ms)
// {
//     uint32_t now = iic_get_tick_ms();

//     if (now >= deadline_ms)
//     {
//         *remain_ms = 0;
//         return IIC_TIMEOUT;
//     }

//     *remain_ms = deadline_ms - now;
//     return IIC_OK;
// }

// iic_status_t iic_is_device_ready(uint32_t i2c_periph,
//                                  uint8_t dev_addr_7bit,
//                                  uint32_t timeout_ms)
// {
//     iic_status_t ret;
//     uint32_t deadline_ms;
//     uint32_t remain_ms;

//     deadline_ms = iic_get_tick_ms() + timeout_ms;

//     /* ACK 位置配置（保持你原来的） */
//     i2c_ackpos_config(i2c_periph, I2C_ACKPOS_CURRENT);

//     /* START */
//     i2c_start_on_bus(i2c_periph);

//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         i2c_stop_on_bus(i2c_periph);
//         return ret;
//     }

//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         i2c_stop_on_bus(i2c_periph);
//         return ret;
//     }

//     /* 发送地址（写方向） */
//     i2c_master_addressing(i2c_periph,
//                           (uint32_t)(dev_addr_7bit << 1),
//                           I2C_TRANSMITTER);

//     /* 等待 ADDRSEND 或 AERR */
//     while (1)
//     {
//         /* 成功：设备应答 */
//         if (i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
//         {
//             i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);
//             i2c_stop_on_bus(i2c_periph);
//             return IIC_OK;
//         }

//         /* 失败：NACK（设备没准备好 / 不存在） */
//         if (i2c_flag_get(i2c_periph, I2C_FLAG_AERR))
//         {
//             i2c_flag_clear(i2c_periph, I2C_FLAG_AERR);
//             i2c_stop_on_bus(i2c_periph);
//             return IIC_ERROR;
//         }

//         /* 超时控制（关键） */
//         ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//         if (ret != IIC_OK)
//         {
//             i2c_stop_on_bus(i2c_periph);
//             return IIC_TIMEOUT;
//         }
//     }
// }

// iic_status_t iic_mem_write(uint32_t i2c_periph,
//                            uint8_t dev_addr_7bit,
//                            uint16_t mem_addr,
//                            uint8_t mem_addr_size,
//                            const uint8_t *buf,
//                            uint16_t len,
//                            uint32_t timeout_ms)
// {
//     uint16_t i;
//     iic_status_t ret;
//     uint32_t deadline_ms;
//     uint32_t remain_ms;

//     if ((buf == NULL) || (len == 0U))
//     {
//         return IIC_ERROR_PARAMETER;
//     }

//     timeout_ms = (timeout_ms == 0U) ? IIC_WRITE_PAGE_DEFAULT_TIMEOUT_MS : timeout_ms;

//     deadline_ms = iic_get_tick_ms() + timeout_ms;

//     /* 先检查总线是否忙 */
//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_I2CBSY, RESET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }

//     /* 第一步：发送 START */
//     i2c_start_on_bus(i2c_periph);

//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }

//     /* 第二步：发送器件地址 + 写方向 */
//     i2c_master_addressing(i2c_periph,
//                           (uint32_t)(dev_addr_7bit << 1),
//                           I2C_TRANSMITTER);

//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_ADDSEND, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

//     /* 第三步：发送内存地址 */
//     if (mem_addr_size == IIC_MEM_ADDR_16BIT)
//     {
//         i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr >> 8));

//         ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//         if (ret != IIC_OK)
//         {
//             return ret;
//         }
//         ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
//         if (ret != IIC_OK)
//         {
//             return ret;
//         }
//     }

//     i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr & 0xFF));

//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }

//     /* 第四步：发送数据 */
//     for (i = 0; i < len; i++)
//     {
//         i2c_data_transmit(i2c_periph, buf[i]);

//         ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//         if (ret != IIC_OK)
//         {
//             return ret;
//         }
//         ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
//         if (ret != IIC_OK)
//         {
//             return ret;
//         }
//     }

//     /* 第五步：等待最后一个字节真正发完 */
//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_BTC, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }

//     /* 第六步：发送 STOP */
//     i2c_stop_on_bus(i2c_periph);

//     return IIC_OK;
// }

// iic_status_t iic_mem_read(uint32_t i2c_periph,
//                           uint8_t dev_addr_7bit,
//                           uint16_t mem_addr,
//                           uint8_t mem_addr_size,
//                           uint8_t *buf,
//                           uint16_t len,
//                           uint32_t timeout_ms)
// {
//     uint16_t i;
//     iic_status_t ret;
//     uint32_t deadline_ms;
//     uint32_t remain_ms;

//     if ((buf == NULL) || (len == 0U))
//     {
//         return IIC_ERROR_PARAMETER;
//     }

//     timeout_ms = (timeout_ms == 0U) ? IIC_READ_PIPE_PARA_DEFAULT_TIMEOUT_MS : timeout_ms;

//     deadline_ms = iic_get_tick_ms() + timeout_ms;

//     /* 先检查总线是否忙 */
//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_I2CBSY, RESET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }

//     /* 第一步：写内存地址 */
//     i2c_start_on_bus(i2c_periph);

//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }

//     /* 向dev_addr_7bit发出写信号 */
//     i2c_master_addressing(i2c_periph,
//                           (uint32_t)(dev_addr_7bit << 1),
//                           I2C_TRANSMITTER);

//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_ADDSEND, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     /* 清一下FLAG */
//     i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

//     /* 16位的话就先发高地址 */
//     if (mem_addr_size == IIC_MEM_ADDR_16BIT)
//     {
//         i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr >> 8));

//         ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//         if (ret != IIC_OK)
//         {
//             return ret;
//         }
//         ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
//         if (ret != IIC_OK)
//         {
//             return ret;
//         }
//     }

//     /* 发8位低地址 */
//     i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr & 0xFF));

//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }

//     /* 第二步：重复起始，切换到读 */
//     i2c_start_on_bus(i2c_periph);

//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }

//     /* 对dev_addr_7bit发送读信号 */
//     i2c_master_addressing(i2c_periph,
//                           (uint32_t)(dev_addr_7bit << 1),
//                           I2C_RECEIVER);

//     ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_ADDSEND, SET, remain_ms);
//     if (ret != IIC_OK)
//     {
//         return ret;
//     }
//     i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

//     /* 第三步：读数据 */
//     for (i = 0; i < len; i++)
//     {
//         if (i == (len - 1U))
//         {
//             /* 接收到最后一个数据 不发ACK 发STOP */
//             /* 其他情况 硬件就会在每个字节结束后自动回 ACK */
//             i2c_ack_config(i2c_periph, I2C_ACK_DISABLE);
//             i2c_stop_on_bus(i2c_periph);
//         }

//         ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
//         if (ret != IIC_OK)
//         {
//             /* 超时 把I2C外设的ACK配置恢复成默认正常状态 防止影响下一次通信 */
//             i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
//             return ret;
//         }
//         ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_RBNE, SET, remain_ms);
//         if (ret != IIC_OK)
//         {
//             i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
//             return ret;
//         }

//         buf[i] = i2c_data_receive(i2c_periph);
//     }

//     i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);

//     return IIC_OK;
// }

// void iic_driver_init(iic_driver_t *driver)
// {
//     if (driver == NULL)
//         return;

//     driver->i2c_periph = EEPROM_I2C;
//     driver->pf_iic_init = iic_init;
//     driver->pf_iic_is_ready = iic_is_device_ready;
//     driver->pf_iic_mem_write = iic_mem_write;
//     driver->pf_iic_mem_read = iic_mem_read;
// }

#include "FreeRTOS.h"
#include "task.h"

#include "iic.h"

#include <stddef.h>

static iic_status_t iic_wait_flag_set(uint32_t i2c_periph,
                                      i2c_flag_enum flag,
                                      FlagStatus status,
                                      uint32_t timeout)
{
    TickType_t start_tick = xTaskGetTickCount();
    TickType_t timeout_tick = pdMS_TO_TICKS(timeout);

    while (i2c_flag_get(i2c_periph, flag) != status)
    {
        if ((xTaskGetTickCount() - start_tick) >= timeout_tick)
        {
            return IIC_TIMEOUT;
        }
    }
    return IIC_OK;
}

static uint32_t iic_get_tick_ms(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static iic_status_t iic_get_remaining_timeout(uint32_t deadline_ms,
                                              uint32_t *remain_ms)
{
    uint32_t now = iic_get_tick_ms();

    if (now >= deadline_ms)
    {
        *remain_ms = 0U;
        return IIC_TIMEOUT;
    }

    *remain_ms = deadline_ms - now;
    return IIC_OK;
}

/* ---------------- GPIO mode switch for I2C bus recover ---------------- */

static void iic_bus_gpio_to_od_out(void)
{
    gpio_init(EEPROM_I2C_GPIO_PORT,
              GPIO_MODE_OUT_OD,
              GPIO_OSPEED_50MHZ,
              EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN);

    /* release bus */
    gpio_bit_set(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN);
}

static void iic_bus_gpio_to_af_od(void)
{
    gpio_init(EEPROM_I2C_GPIO_PORT,
              GPIO_MODE_AF_OD,
              GPIO_OSPEED_50MHZ,
              EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN);

    gpio_bit_set(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN);
}

static void iic_bus_delay_us_soft(uint32_t cnt)
{
    volatile uint32_t i, j;
    for (i = 0; i < cnt; i++)
    {
        for (j = 0; j < 20U; j++)
        {
            __NOP();
        }
    }
}

/*
 * 总线恢复：
 * 1. 切 GPIO 开漏输出
 * 2. 若 SDA 被拉低，则手动给 SCL 打 9 个脉冲
 * 3. 人工构造一个 STOP：SDA low -> SCL high -> SDA high
 * 4. 切回 AF_OD
 */
static void iic_bus_recover(uint32_t i2c_periph)
{
    uint32_t i;

    /* 先关 I2C 外设，避免和 GPIO 抢线 */
    i2c_disable(i2c_periph);
    i2c_deinit(i2c_periph);

    iic_bus_gpio_to_od_out();

    /* 先释放两根线 */
    gpio_bit_set(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN);
    iic_bus_delay_us_soft(20U);

    /* 若 SDA 仍然为低，说明可能从机卡住，给 SCL 9 个脉冲 */
    for (i = 0; i < 9U; i++)
    {
        if (gpio_input_bit_get(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SDA_PIN) == SET)
        {
            break;
        }

        gpio_bit_set(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN);
        iic_bus_delay_us_soft(20U);

        gpio_bit_reset(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN);
        iic_bus_delay_us_soft(20U);
    }

    /* 构造 STOP：SDA low -> SCL high -> SDA high */
    gpio_bit_reset(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SDA_PIN);
    iic_bus_delay_us_soft(20U);

    gpio_bit_set(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN);
    iic_bus_delay_us_soft(20U);

    gpio_bit_set(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SDA_PIN);
    iic_bus_delay_us_soft(20U);

    /* 切回 I2C 复用开漏 */
    iic_bus_gpio_to_af_od();
}

iic_status_t iic_init(uint32_t i2c_periph, uint32_t clock_speed)
{
    /* 1. 时钟使能：在 rcu_config() 完成 */
    /* 2. GPIO 基础配置：在 gpio_config() 完成 */

    /* 3. 先恢复总线，防止上次异常后 BUSY 卡死 */
    iic_bus_recover(i2c_periph);

    /* 4. 再初始化 I2C 外设 */
    i2c_deinit(i2c_periph);

    i2c_clock_config(i2c_periph, clock_speed, I2C_DTCY_2);
    i2c_mode_addr_config(i2c_periph, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
    i2c_ackpos_config(i2c_periph, I2C_ACKPOS_CURRENT);
    i2c_enable(i2c_periph);

    return IIC_OK;
}

void iic_deinit(uint32_t i2c_periph)
{
    i2c_deinit(i2c_periph);
}

iic_status_t iic_is_device_ready(uint32_t i2c_periph,
                                 uint8_t dev_addr_7bit,
                                 uint32_t timeout_ms)
{
    iic_status_t ret;
    uint32_t deadline_ms;
    uint32_t remain_ms;

    timeout_ms = (timeout_ms == 0U) ? 5U : timeout_ms;
    deadline_ms = iic_get_tick_ms() + timeout_ms;

    i2c_ackpos_config(i2c_periph, I2C_ACKPOS_CURRENT);
    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);

    /* 先等 BUSY 释放；若不释放，尝试恢复一次 */
    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        return ret;
    }

    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_I2CBSY, RESET, remain_ms);
    if (ret != IIC_OK)
    {
        iic_bus_recover(i2c_periph);

        /* recover 后重新初始化 IIC 外设 */
        i2c_deinit(i2c_periph);
        i2c_clock_config(i2c_periph, E2PROM_I2C_SPEED_CLOCK, I2C_DTCY_2);
        i2c_mode_addr_config(i2c_periph, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0);
        i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
        i2c_ackpos_config(i2c_periph, I2C_ACKPOS_CURRENT);
        i2c_enable(i2c_periph);

        ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != IIC_OK)
        {
            return ret;
        }

        ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_I2CBSY, RESET, remain_ms);
        if (ret != IIC_OK)
        {
            return ret;
        }
    }

    i2c_start_on_bus(i2c_periph);

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }

    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }

    i2c_master_addressing(i2c_periph,
                          (uint32_t)(dev_addr_7bit << 1),
                          I2C_TRANSMITTER);

    while (1)
    {
        if (i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
        {
            i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);
            i2c_stop_on_bus(i2c_periph);
            return IIC_OK;
        }

        if (i2c_flag_get(i2c_periph, I2C_FLAG_AERR))
        {
            i2c_flag_clear(i2c_periph, I2C_FLAG_AERR);
            goto ERR_EXIT;
        }

        ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != IIC_OK)
        {
            goto ERR_EXIT;
        }
    }

ERR_EXIT:
    i2c_stop_on_bus(i2c_periph);
    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
    return ret;
}

iic_status_t iic_mem_write(uint32_t i2c_periph,
                           uint8_t dev_addr_7bit,
                           uint16_t mem_addr,
                           uint8_t mem_addr_size,
                           const uint8_t *buf,
                           uint16_t len,
                           uint32_t timeout_ms)
{
    uint16_t i;
    iic_status_t ret;
    uint32_t deadline_ms;
    uint32_t remain_ms;

    if ((buf == NULL) || (len == 0U))
    {
        return IIC_ERROR_PARAMETER;
    }

    if ((mem_addr_size != IIC_MEM_ADDR_8BIT) &&
        (mem_addr_size != IIC_MEM_ADDR_16BIT))
    {
        return IIC_ERROR_PARAMETER;
    }

    timeout_ms = (timeout_ms == 0U) ? IIC_WRITE_PAGE_DEFAULT_TIMEOUT_MS : timeout_ms;
    deadline_ms = iic_get_tick_ms() + timeout_ms;

    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
    i2c_ackpos_config(i2c_periph, I2C_ACKPOS_CURRENT);

    /* 先检查总线是否忙；若 BUSY，则恢复一次 */
    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        return ret;
    }

    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_I2CBSY, RESET, remain_ms);
    if (ret != IIC_OK)
    {
        iic_bus_recover(i2c_periph);

        /* 恢复完要重新初始化 I2C 外设 */
        i2c_deinit(i2c_periph);
        i2c_clock_config(i2c_periph, E2PROM_I2C_SPEED_CLOCK, I2C_DTCY_2);
        i2c_mode_addr_config(i2c_periph, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
        i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
        i2c_ackpos_config(i2c_periph, I2C_ACKPOS_CURRENT);
        i2c_enable(i2c_periph);

        ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != IIC_OK)
        {
            return ret;
        }

        ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_I2CBSY, RESET, remain_ms);
        if (ret != IIC_OK)
        {
            return ret;
        }
    }

    /* START */
    i2c_start_on_bus(i2c_periph);

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }

    /* dev addr + write */
    i2c_master_addressing(i2c_periph,
                          (uint32_t)(dev_addr_7bit << 1),
                          I2C_TRANSMITTER);

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_ADDSEND, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    /* mem addr */
    if (mem_addr_size == IIC_MEM_ADDR_16BIT)
    {
        i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr >> 8));

        ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != IIC_OK)
        {
            goto ERR_EXIT;
        }
        ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
        if (ret != IIC_OK)
        {
            goto ERR_EXIT;
        }
    }

    i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr & 0xFF));

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }

    /* data */
    for (i = 0; i < len; i++)
    {
        i2c_data_transmit(i2c_periph, buf[i]);

        ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != IIC_OK)
        {
            goto ERR_EXIT;
        }
        ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
        if (ret != IIC_OK)
        {
            goto ERR_EXIT;
        }
    }

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_BTC, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }

    i2c_stop_on_bus(i2c_periph);
    return IIC_OK;

ERR_EXIT:
    i2c_stop_on_bus(i2c_periph);
    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
    return ret;
}

iic_status_t iic_mem_read(uint32_t i2c_periph,
                          uint8_t dev_addr_7bit,
                          uint16_t mem_addr,
                          uint8_t mem_addr_size,
                          uint8_t *buf,
                          uint16_t len,
                          uint32_t timeout_ms)
{
    uint16_t i;
    iic_status_t ret;
    uint32_t deadline_ms;
    uint32_t remain_ms;

    if ((buf == NULL) || (len == 0U))
    {
        return IIC_ERROR_PARAMETER;
    }

    if ((mem_addr_size != IIC_MEM_ADDR_8BIT) &&
        (mem_addr_size != IIC_MEM_ADDR_16BIT))
    {
        return IIC_ERROR_PARAMETER;
    }

    timeout_ms = (timeout_ms == 0U) ? IIC_READ_PIPE_PARA_DEFAULT_TIMEOUT_MS : timeout_ms;
    deadline_ms = iic_get_tick_ms() + timeout_ms;

    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
    i2c_ackpos_config(i2c_periph, I2C_ACKPOS_CURRENT);

    /* 先等总线空闲；若 BUSY，则恢复一次 */
    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        return ret;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_I2CBSY, RESET, remain_ms);
    if (ret != IIC_OK)
    {
        iic_bus_recover(i2c_periph);

        i2c_deinit(i2c_periph);
        i2c_clock_config(i2c_periph, E2PROM_I2C_SPEED_CLOCK, I2C_DTCY_2);
        i2c_mode_addr_config(i2c_periph, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
        i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
        i2c_ackpos_config(i2c_periph, I2C_ACKPOS_CURRENT);
        i2c_enable(i2c_periph);

        ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != IIC_OK)
        {
            return ret;
        }
        ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_I2CBSY, RESET, remain_ms);
        if (ret != IIC_OK)
        {
            return ret;
        }
    }

    /* step1: write mem addr */
    i2c_start_on_bus(i2c_periph);

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }

    i2c_master_addressing(i2c_periph,
                          (uint32_t)(dev_addr_7bit << 1),
                          I2C_TRANSMITTER);

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_ADDSEND, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    if (mem_addr_size == IIC_MEM_ADDR_16BIT)
    {
        i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr >> 8));

        ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != IIC_OK)
        {
            goto ERR_EXIT;
        }
        ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
        if (ret != IIC_OK)
        {
            goto ERR_EXIT;
        }
    }

    i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr & 0xFF));

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }

    /* repeated start */
    i2c_start_on_bus(i2c_periph);

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }

    i2c_master_addressing(i2c_periph,
                          (uint32_t)(dev_addr_7bit << 1),
                          I2C_RECEIVER);

    ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_ADDSEND, SET, remain_ms);
    if (ret != IIC_OK)
    {
        goto ERR_EXIT;
    }
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    /* read bytes */
    for (i = 0; i < len; i++)
    {
        if (i == (len - 1U))
        {
            i2c_ack_config(i2c_periph, I2C_ACK_DISABLE);
            i2c_stop_on_bus(i2c_periph);
        }

        ret = iic_get_remaining_timeout(deadline_ms, &remain_ms);
        if (ret != IIC_OK)
        {
            goto ERR_EXIT;
        }
        ret = iic_wait_flag_set(i2c_periph, I2C_FLAG_RBNE, SET, remain_ms);
        if (ret != IIC_OK)
        {
            goto ERR_EXIT;
        }

        buf[i] = i2c_data_receive(i2c_periph);
    }

    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
    return IIC_OK;

ERR_EXIT:
    i2c_stop_on_bus(i2c_periph);
    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
    return ret;
}

void iic_driver_init(iic_driver_t *driver)
{
    if (driver == NULL)
    {
        return;
    }

    driver->i2c_periph = EEPROM_I2C;
    driver->pf_iic_init = iic_init;
    driver->pf_iic_is_ready = iic_is_device_ready;
    driver->pf_iic_mem_write = iic_mem_write;
    driver->pf_iic_mem_read = iic_mem_read;
}