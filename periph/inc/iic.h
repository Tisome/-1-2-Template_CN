#ifndef __IIC_H
#define __IIC_H

#include "gd32f30x.h"

#include "gpio.h"
#include "rcu.h"

#include <stdint.h>

/* I2C 引脚定义 */
#define EEPROM_I2C I2C0

/* EEPROM 常用地址宽度 */
#define IIC_MEM_ADDR_8BIT      1U
#define IIC_MEM_ADDR_16BIT     2U

#define E2PROM_I2C_SPEED_CLOCK 100000U

#ifndef IIC_DEFAULT_TIMEOUT_MS
#define IIC_WRITE_PAGE_DEFAULT_TIMEOUT_MS     10U
#define IIC_READ_PIPE_PARA_DEFAULT_TIMEOUT_MS 50U
#endif

typedef enum {
    IIC_OK = 0,
    IIC_ERROR,
    IIC_BUSY,
    IIC_TIMEOUT,
    IIC_ERROR_PARAMETER
} iic_status_t;

typedef struct
{
    uint32_t i2c_periph;

    iic_status_t (*pf_iic_init)(uint32_t i2c_periph,
                                uint32_t clock_speed);

    iic_status_t (*pf_iic_is_ready)(uint32_t i2c_periph,
                                    uint8_t dev_addr,
                                    uint32_t timeout_ms);

    iic_status_t (*pf_iic_mem_write)(uint32_t i2c_periph,
                                     uint8_t dev_addr,
                                     uint16_t mem_addr,
                                     uint8_t mem_addr_size,
                                     const uint8_t *buf,
                                     uint16_t len,
                                     uint32_t timeout_ms);

    iic_status_t (*pf_iic_mem_read)(uint32_t i2c_periph,
                                    uint8_t dev_addr,
                                    uint16_t mem_addr,
                                    uint8_t mem_addr_size,
                                    uint8_t *buf,
                                    uint16_t len,
                                    uint32_t timeout_ms);
} iic_driver_t;

iic_status_t iic_init(uint32_t i2c_periph,
                      uint32_t clock_speed);

iic_status_t iic_is_device_ready(uint32_t i2c_periph,
                                 uint8_t dev_addr_7bit,
                                 uint32_t timeout_ms);

iic_status_t iic_mem_write(uint32_t i2c_periph,
                           uint8_t dev_addr_7bit,
                           uint16_t mem_addr,
                           uint8_t mem_addr_size,
                           const uint8_t *buf,
                           uint16_t len,
                           uint32_t timeout_ms);

iic_status_t iic_mem_read(uint32_t i2c_periph,
                          uint8_t dev_addr_7bit,
                          uint16_t mem_addr,
                          uint8_t mem_addr_size,
                          uint8_t *buf,
                          uint16_t len,
                          uint32_t timeout_ms);

void iic_driver_init(iic_driver_t *driver);

#endif