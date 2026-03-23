#ifndef __IIC_H
#define __IIC_H

#include "gd32f30x.h"

#include "gpio.h"
#include "rcu.h"

#include <stdint.h>

/* I2C 引脚定义 */
#define EEPROM_I2C I2C0

/* EEPROM 常用地址宽度 */
#define IIC_MEM_ADDR_8BIT  1U
#define IIC_MEM_ADDR_16BIT 2U

#ifndef IIC_DEFAULT_TIMEOUT
#define IIC_DEFAULT_TIMEOUT 100000U
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
    void (*pf_init)(uint32_t i2c_periph, uint32_t clock_speed);
    iic_status_t (*pf_is_ready)(uint32_t i2c_periph, uint8_t dev_addr);
    iic_status_t (*pf_mem_write)(uint32_t i2c_periph, uint8_t dev_addr, uint16_t mem_addr,
                                 uint8_t mem_addr_size, const uint8_t *buf, uint16_t len);
    iic_status_t (*pf_mem_read)(uint32_t i2c_periph, uint8_t dev_addr, uint16_t mem_addr,
                                uint8_t mem_addr_size, uint8_t *buf, uint16_t len);
} iic_driver_t;

void iic_init(uint32_t i2c_periph, uint32_t clock_speed);
void iic_deinit(uint32_t i2c_periph);

iic_status_t iic_is_device_ready(uint32_t i2c_periph, uint8_t dev_addr_7bit);

iic_status_t iic_mem_write(uint32_t i2c_periph,
                           uint8_t dev_addr_7bit,
                           uint16_t mem_addr,
                           uint8_t mem_addr_size,
                           const uint8_t *buf,
                           uint16_t len);

iic_status_t iic_mem_read(uint32_t i2c_periph,
                          uint8_t dev_addr_7bit,
                          uint16_t mem_addr,
                          uint8_t mem_addr_size,
                          uint8_t *buf,
                          uint16_t len);

void iic_driver_init(iic_driver_t *driver);

#endif