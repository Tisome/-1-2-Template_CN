#ifndef __AT24CXX_DRIVER_H
#define __AT24CXX_DRIVER_H

#include "iic.h"

#include "app_config.h"

#include <stdint.h>

/* 常见器件默认配置（可按板级硬件修改） */
#if E2PROM_AT24C32
#define AT24CXX_DEFAULT_I2C_ADDR_7BIT             0x50U              // 7bit的at24cxx器件iic地址
#define AT24CXX_DEFAULT_PAGE_SIZE                 32U                // 内部数据页大小(B)
#define AT24CXX_DEFAULT_MEM_ADDR_SIZE             IIC_MEM_ADDR_16BIT // 内部数据地址长度
#define AT24CXX_DEFAULT_MEM_SIZE_BYTES            4096U              // 内部数据存储容量
#define AT24CXX_DEFAULT_WRITE_PAGE_TIMEOUT_MS     20U                // 写超时时间
#define AT24CXX_DEFAULT_READY_TIMEOUT_MS          20U                // ready测试超时时间
#define AT24CXX_DEFAULT_READ_PIPE_PARA_TIMEOUT_MS 100U               // 读pipe para大小数据的超时时间

#elif E2PROM_AT24C08
#define AT24CXX_DEFAULT_I2C_ADDR_7BIT             0x50U
#define AT24CXX_DEFAULT_PAGE_SIZE                 16U
#define AT24CXX_DEFAULT_MEM_ADDR_SIZE             IIC_MEM_ADDR_8BIT
#define AT24CXX_DEFAULT_MEM_SIZE_BYTES            1024U
#define AT24CXX_DEFAULT_WRITE_PAGE_TIMEOUT_MS     10U
#define AT24CXX_DEFAULT_READY_TIMEOUT_MS          10U
#define AT24CXX_DEFAULT_READ_PIPE_PARA_TIMEOUT_MS 50U
#endif

typedef enum {
    AT24CXX_OK              = 0, /* R/W SUCCESSFULLY */
    AT24CXX_BUSY            = 1, /* AT24CXX I2C BUSY */
    AT24CXX_TIMEOUT         = 2, /* AT24CXX I2C TIMEOUT */
    AT24CXX_ERROR           = 3, /* AT24CXX I2C ERROR WITHOUT ANY CASE MATCHED */
    AT24CXX_ERROR_RESOURCE  = 4, /* AT24CXX RESOURCES DOESN'T EXIST  */
    AT24CXX_ERROR_PARAMETER = 5, /* AT24CXX INPUT PARAMETER WRONG */
    AT24CXX_ERROR_NO_MEMORY = 6, /* AT24CXX HAS NO REST MEMORY FOR WRITE */
    AT24CXX_RESERVED        = 7
} at24cxx_status_t;

/*
 * @brief AT24Cxx 设备描述
 *
 * mem_addr_size: IIC_MEM_ADDR_8BIT / IIC_MEM_ADDR_16BIT
 * mem_size_bytes: 可选边界保护，0 表示不做边界检查
 */
typedef struct
{
    uint16_t i2c_addr_7bit;             // i2c设备地址
    uint16_t page_size;                 // 页内字节数量
    uint16_t mem_addr_size;             // 内部地址长度
    uint16_t mem_size_bytes;            // 内部存储字节容量
    uint32_t write_cycle_timeout_ms;    // write一页的超时时间
    uint32_t ready_timeout_ms;          // ready查询的超时时间
    uint32_t read_pipe_para_timeout_ms; // 读一整个pipe para的超时时间
} at24cxx_dev_info_t;

at24cxx_status_t at24cxx_init_default(at24cxx_dev_info_t *dev);

at24cxx_status_t at24cxx_wait_ready(const at24cxx_dev_info_t *dev,
                                    const iic_driver_t *iic_driver);

at24cxx_status_t at24cxx_read(const at24cxx_dev_info_t *dev,
                              const iic_driver_t *iic_driver,
                              uint16_t mem_addr,
                              uint8_t *buf,
                              uint16_t len);

at24cxx_status_t at24cxx_write(const at24cxx_dev_info_t *dev,
                               const iic_driver_t *iic_driver,
                               uint16_t mem_addr,
                               const uint8_t *buf,
                               uint16_t len);

typedef struct
{
    at24cxx_status_t (*pf_init)(at24cxx_dev_info_t *dev);

    at24cxx_status_t (*pf_is_ready)(const at24cxx_dev_info_t *dev,
                                    const iic_driver_t *iic_driver);

    at24cxx_status_t (*pf_read)(const at24cxx_dev_info_t *dev,
                                const iic_driver_t *iic_driver,
                                uint16_t mem_addr,
                                uint8_t *buf,
                                uint16_t len);

    at24cxx_status_t (*pf_write)(const at24cxx_dev_info_t *dev,
                                 const iic_driver_t *iic_driver,
                                 uint16_t mem_addr,
                                 const uint8_t *buf,
                                 uint16_t len);

} at24cxx_driver_t;

/************************* at24cxx driver struct *************************/

typedef struct
{
    iic_driver_t *p_iic_driver_instance;
    at24cxx_dev_info_t *p_at24cxx_dev_info;
    at24cxx_driver_t *p_at24cxx_driver_instance;

} at24cxx_handler_t;

/*
 * Readable aliases for the AT24Cxx low-level driver layer.
 * These aliases do not change the underlying implementation and are safe to
 * adopt gradually in higher-level code.
 */
typedef at24cxx_dev_info_t eeprom_chip_config_t;
typedef at24cxx_driver_t eeprom_chip_driver_t;
typedef at24cxx_handler_t eeprom_chip_link_t;

void at24cxx_driver_init(at24cxx_driver_t *driver);

#endif
