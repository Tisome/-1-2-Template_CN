#include "iic.h"

#include <stddef.h>

static iic_status_t iic_wait_flag_set(uint32_t i2c_periph,
                                      i2c_flag_enum flag,
                                      FlagStatus status,
                                      uint32_t timeout)
{
    while (i2c_flag_get(i2c_periph, flag) != status)
    {
        if (timeout-- == 0U)
        {
            return IIC_TIMEOUT;
        }
    }
    return IIC_OK;
}

void iic_init(uint32_t i2c_periph, uint32_t clock_speed)
{
    /* 1. 时钟使能 在rcu.c中完成*/

    /* 2. GPIO 配置：I2C 必须 AF_OD 在gpio.c中完成*/

    /* 3. I2C 复位 */
    i2c_deinit(i2c_periph);

    /* 4. I2C 参数配置 */
    i2c_clock_config(i2c_periph, clock_speed, I2C_DTCY_2);
    i2c_mode_addr_config(i2c_periph, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
    i2c_enable(i2c_periph);
    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
}

void iic_deinit(uint32_t i2c_periph)
{
    i2c_deinit(i2c_periph);
}

iic_status_t iic_is_device_ready(uint32_t i2c_periph,
                                 uint8_t dev_addr_7bit)
{
    uint32_t timeout = IIC_DEFAULT_TIMEOUT;

    i2c_ackpos_config(i2c_periph, I2C_ACKPOS_CURRENT);
    i2c_start_on_bus(i2c_periph);

    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;

    i2c_master_addressing(i2c_periph, (uint32_t)(dev_addr_7bit << 1), I2C_TRANSMITTER);

    timeout = IIC_DEFAULT_TIMEOUT;
    while (!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
    {
        if (i2c_flag_get(i2c_periph, I2C_FLAG_AERR))
        {
            i2c_stop_on_bus(i2c_periph);
            i2c_flag_clear(i2c_periph, I2C_FLAG_AERR);
            return IIC_ERROR;
        }

        if (timeout-- == 0U)
        {
            i2c_stop_on_bus(i2c_periph);
            return IIC_TIMEOUT;
        }
    }

    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);
    i2c_stop_on_bus(i2c_periph);

    return IIC_OK;
}

iic_status_t iic_mem_write(uint32_t i2c_periph,
                           uint8_t dev_addr_7bit,
                           uint16_t mem_addr,
                           uint8_t mem_addr_size,
                           const uint8_t *buf,
                           uint16_t len)
{
    uint16_t i;
    uint32_t timeout = IIC_DEFAULT_TIMEOUT;

    if ((buf == NULL) || (len == 0))
        return IIC_ERROR_PARAMETER;

    i2c_start_on_bus(i2c_periph);
    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;

    i2c_master_addressing(i2c_periph, (uint32_t)(dev_addr_7bit << 1), I2C_TRANSMITTER);
    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_ADDSEND, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    /* 发送内存地址 */
    if (mem_addr_size == IIC_MEM_ADDR_16BIT)
    {
        i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr >> 8));
        if (iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, timeout) != IIC_OK)
            return IIC_TIMEOUT;
    }

    i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr & 0xFF));
    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;

    /* 发送数据 */
    for (i = 0; i < len; i++)
    {
        i2c_data_transmit(i2c_periph, buf[i]);
        if (iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, timeout) != IIC_OK)
            return IIC_TIMEOUT;
    }

    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_BTC, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;

    i2c_stop_on_bus(i2c_periph);

    return IIC_OK;
}

iic_status_t iic_mem_read(uint32_t i2c_periph,
                          uint8_t dev_addr_7bit,
                          uint16_t mem_addr,
                          uint8_t mem_addr_size,
                          uint8_t *buf,
                          uint16_t len)
{
    uint16_t i;
    uint32_t timeout = IIC_DEFAULT_TIMEOUT;

    if ((buf == NULL) || (len == 0))
        return IIC_ERROR_PARAMETER;

    /* 第一步：写内存地址 */
    i2c_start_on_bus(i2c_periph);
    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;

    i2c_master_addressing(i2c_periph, (uint32_t)(dev_addr_7bit << 1), I2C_TRANSMITTER);
    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_ADDSEND, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    if (mem_addr_size == IIC_MEM_ADDR_16BIT)
    {
        i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr >> 8));
        if (iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, timeout) != IIC_OK)
            return IIC_TIMEOUT;
    }

    i2c_data_transmit(i2c_periph, (uint8_t)(mem_addr & 0xFF));
    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_TBE, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;

    /* 第二步：重复起始，切换读 */
    i2c_start_on_bus(i2c_periph);
    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_SBSEND, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;

    i2c_master_addressing(i2c_periph, (uint32_t)(dev_addr_7bit << 1), I2C_RECEIVER);
    if (iic_wait_flag_set(i2c_periph, I2C_FLAG_ADDSEND, SET, timeout) != IIC_OK)
        return IIC_TIMEOUT;
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    /* 读数据 */
    for (i = 0; i < len; i++)
    {
        if (i == (len - 1))
        {
            i2c_ack_config(i2c_periph, I2C_ACK_DISABLE);
            i2c_stop_on_bus(i2c_periph);
        }

        if (iic_wait_flag_set(i2c_periph, I2C_FLAG_RBNE, SET, timeout) != IIC_OK)
            return IIC_TIMEOUT;

        buf[i] = i2c_data_receive(i2c_periph);
    }

    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);

    return IIC_OK;
}

void iic_driver_init(iic_driver_t *driver)
{
    if (driver == NULL)
        return;

    driver->i2c_periph = EEPROM_I2C;
    driver->pf_init = iic_init;
    driver->pf_is_ready = iic_is_device_ready;
    driver->pf_mem_write = iic_mem_write;
    driver->pf_mem_read = iic_mem_read;
}