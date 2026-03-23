#include "at24cxx_driver.h"
#include "iic.h"

#include "elog.h"

#include <stddef.h>

static at24cxx_status_t at24cxx_status_from_iic_status(iic_status_t iic_ret)
{
    switch (iic_ret)
    {
    case (IIC_OK): {
        return AT24CXX_OK;
        break;
    }
    case (IIC_BUSY): {
        log_e("at24cxx r/w busy");
        return AT24CXX_BUSY;
        break;
    }
    case (IIC_TIMEOUT): {
        log_e("at24cxx r/w timeout");
        return AT24CXX_TIMEOUT;
        break;
    }
    case (IIC_ERROR): {
        log_e("at24cxx r/w no matched error");
        return AT24CXX_ERROR;
        break;
    }
    case (IIC_ERROR_PARAMETER): {
        log_e("at24cxx i2c input parameter error");
        return AT24CXX_ERROR_RESOURCE;
        break;
    }
    default: {
        log_e("i2c return status error");
        return AT24CXX_ERROR;
        break;
    }
    }
}

static at24cxx_status_t at24cxx_check_args(const at24cxx_dev_info_t *dev,
                                           uint16_t mem_addr,
                                           uint16_t len)
{
    if (dev == NULL)
    {
        log_e("AT24CXX DEV INFO NO EXIST");
        return AT24CXX_ERROR_RESOURCE;
    }

    if ((dev->mem_addr_size != IIC_MEM_ADDR_8BIT) &&
        (dev->mem_addr_size != IIC_MEM_ADDR_16BIT))
    {
        log_e("AT24CXX MEM ADDR SIZE ERROR");
        return AT24CXX_ERROR_RESOURCE;
    }

    if (dev->page_size == 0U)
    {
        log_e("AT24CXX PAGE SIZE ERROR");
        return AT24CXX_ERROR_RESOURCE;
    }

    if (len == 0U)
    {
        log_e("AT24CXX PARA LEN ERROR");
        return AT24CXX_ERROR_PARAMETER;
    }

    if (dev->mem_size_bytes > 0U)
    {
        uint32_t end_addr = (uint32_t)mem_addr + (uint32_t)len;
        if (end_addr > dev->mem_size_bytes)
        {
            log_e("AT24CXX address overflow: mem_addr=%u len=%u end=%lu mem_size=%u",
                  mem_addr,
                  len,
                  end_addr,
                  dev->mem_size_bytes);
            return AT24CXX_ERROR_PARAMETER;
        }
    }

    return AT24CXX_OK;
}

static at24cxx_status_t at24cxx_check_iic_driver(const iic_driver_t *iic_driver)
{
    if ((iic_driver == NULL) || (iic_driver->hi2c == NULL))
    {
        log_e("AT24CXX I2C driver resource error");
        return AT24CXX_ERROR_RESOURCE;
    }

    if ((iic_driver->pf_iic_is_ready == NULL) || (iic_driver->pf_iic_mem_read == NULL) ||
        (iic_driver->pf_iic_mem_write == NULL))
    {
        log_e("AT24CXX I2C driver function pointer error");
        return AT24CXX_ERROR_RESOURCE;
    }

    return AT24CXX_OK;
}

at24cxx_status_t at24cxx_init_default(at24cxx_dev_info_t *dev)
{
    if (dev == NULL)
    {
        log_e("INPUT AT24CXX DEV INFO POINTER IS NULL");
        return AT24CXX_ERROR;
    }

    dev->i2c_addr_7bit = AT24CXX_DEFAULT_I2C_ADDR_7BIT;
    dev->page_size = AT24CXX_DEFAULT_PAGE_SIZE;
    dev->mem_addr_size = AT24CXX_DEFAULT_MEM_ADDR_SIZE;
    dev->mem_size_bytes = AT24CXX_DEFAULT_MEM_SIZE_BYTES;
    dev->write_cycle_timeout_ms = AT24CXX_DEFAULT_WRITE_CYCLE_TIMEOUTMS;

    return AT24CXX_OK;
}

at24cxx_status_t at24cxx_is_ready(const at24cxx_dev_info_t *dev,
                                  const iic_driver_t *iic_driver,
                                  uint32_t trials)
{
    if (dev == NULL)
    {
        log_e("AT24CXX DEV INFO NO EXIST");
        return AT24CXX_ERROR_RESOURCE;
    }

    at24cxx_status_t at24cxx_ret = at24cxx_check_iic_driver(iic_driver);
    if (at24cxx_ret != AT24CXX_OK)
    {
        return at24cxx_ret;
    }

    if (trials == 0U)
    {
        trials = 1U;
    }

    iic_status_t iic_ret = iic_driver->pf_iic_is_ready(iic_driver->hi2c,
                                                       dev->i2c_addr_7bit,
                                                       trials,
                                                       dev->write_cycle_timeout_ms);
    at24cxx_ret = at24cxx_status_from_iic_status(iic_ret);
    if (at24cxx_ret != AT24CXX_OK)
    {
        log_e("AT24CXX IS NOT READY");
    }
    return at24cxx_ret;
}

at24cxx_status_t at24cxx_read(const at24cxx_dev_info_t *dev,
                              const iic_driver_t *iic_driver,
                              uint16_t mem_addr,
                              uint8_t *buf,
                              uint16_t len)
{
    if (buf == NULL)
    {
        log_e("AT24CXX BUF IS NULL");
        return AT24CXX_ERROR_PARAMETER;
    }

    at24cxx_status_t at24cxx_ret = at24cxx_check_args(dev, mem_addr, len);
    if (at24cxx_ret != AT24CXX_OK)
    {
        log_e("AT24CXX CHECK ARGS ERROR");
        return at24cxx_ret;
    }

    at24cxx_ret = at24cxx_check_iic_driver(iic_driver);
    if (at24cxx_ret != AT24CXX_OK)
    {
        return at24cxx_ret;
    }

    iic_status_t iic_ret = iic_driver->pf_iic_mem_read(iic_driver->hi2c,
                                                       dev->i2c_addr_7bit,
                                                       mem_addr,
                                                       dev->mem_addr_size,
                                                       buf,
                                                       len,
                                                       dev->write_cycle_timeout_ms);

    at24cxx_ret = at24cxx_status_from_iic_status(iic_ret);
    if (at24cxx_ret != AT24CXX_OK)
    {
        log_e("AT24CXX READ ERROR");
    }
    return at24cxx_ret;
}

at24cxx_status_t at24cxx_write(const at24cxx_dev_info_t *dev,
                               const iic_driver_t *iic_driver,
                               uint16_t mem_addr,
                               const uint8_t *buf,
                               uint16_t len)
{
    if (buf == NULL)
    {
        log_e("AT24CXX BUF IS NULL");
        return AT24CXX_ERROR_PARAMETER;
    }

    iic_status_t iic_ret;
    at24cxx_status_t at24cxx_ret = at24cxx_check_args(dev, mem_addr, len);
    if (at24cxx_ret != AT24CXX_OK)
    {
        log_e("AT24CXX CHECK ARGS ERROR");
        return at24cxx_ret;
    }

    at24cxx_ret = at24cxx_check_iic_driver(iic_driver);
    if (at24cxx_ret != AT24CXX_OK)
    {
        return at24cxx_ret;
    }

    uint16_t cur_addr = mem_addr; // 当前写的地址
    uint16_t done = 0U;           // 目前已经写了的数据长度

    while (done < len) // 当数据还没有写完的时候
    {
        uint16_t page_offset = (uint16_t)(cur_addr % dev->page_size);  // 本页开始写的时候的页内偏移
        uint16_t page_left = (uint16_t)(dev->page_size - page_offset); // 本页剩下可以写的数据长度
        uint16_t remain = (uint16_t)(len - done);                      // 整个数据包的未写的剩下长度
        uint16_t chunk = (page_left < remain) ? page_left : remain;    // 在本页剩余长度和整个数据包的剩下长度中选一个更小的，即为当前要写的数据长度

        iic_ret = iic_driver->pf_iic_mem_write(iic_driver->hi2c,
                                               dev->i2c_addr_7bit,
                                               cur_addr,
                                               dev->mem_addr_size,
                                               &buf[done],
                                               chunk,
                                               dev->write_cycle_timeout_ms);

        at24cxx_ret = at24cxx_status_from_iic_status(iic_ret);
        if (at24cxx_ret != AT24CXX_OK)
        {
            log_e("AT24CXX WRITE ERROR");
            return at24cxx_ret;
        }

        iic_ret = iic_driver->pf_iic_is_ready(iic_driver->hi2c,
                                              dev->i2c_addr_7bit,
                                              5U,
                                              dev->write_cycle_timeout_ms);

        at24cxx_ret = at24cxx_status_from_iic_status(iic_ret);
        if (at24cxx_ret != AT24CXX_OK)
        {
            log_e("AT24CXX WRITE ERROR");
            return at24cxx_ret;
        }

        cur_addr = (uint16_t)(cur_addr + chunk);
        done = (uint16_t)(done + chunk);
    }

    return AT24CXX_OK;
}

void at24cxx_driver_init(at24cxx_driver_t *driver)
{
    if (driver == NULL)
    {
        return;
    }

    driver->pf_init = at24cxx_init_default;
    driver->pf_is_ready = at24cxx_is_ready;
    driver->pf_read = at24cxx_read;
    driver->pf_write = at24cxx_write;
}
