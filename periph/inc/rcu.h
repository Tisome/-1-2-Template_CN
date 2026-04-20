#ifndef __RCU_H
#define __RCU_H

#include "gd32f30x.h"

/*==================== GPIO Clock ====================*/
#define RCU_GPIO_FPGA_INT_PORT      RCU_GPIOA
#define RCU_GPIO_FPGA_START_PORT    RCU_GPIOA

#define RCU_GPIO_FPGA_SPI_NSS_PORT  RCU_GPIOB
#define RCU_GPIO_FPGA_SPI_SCK_PORT  RCU_GPIOB
#define RCU_GPIO_FPGA_SPI_MISO_PORT RCU_GPIOB
#define RCU_GPIO_FPGA_SPI_MOSI_PORT RCU_GPIOB

#if CCT6 || CGT6
#define RCU_GPIO_KEY1_PORT RCU_GPIOB
#define RCU_GPIO_KEY2_PORT RCU_GPIOA
#define RCU_GPIO_KEY3_PORT RCU_GPIOB
#define RCU_GPIO_KEY4_PORT RCU_GPIOB
#elif RCT6
#define RCU_GPIO_KEY1_PORT RCU_GPIOA
#define RCU_GPIO_KEY2_PORT RCU_GPIOA
#define RCU_GPIO_KEY3_PORT RCU_GPIOA
#define RCU_GPIO_KEY4_PORT RCU_GPIOA

#endif

#define RCU_GPIO_EEPROM_I2C_PORT RCU_GPIOB

#define RCU_GPIO_USART0_PORT     RCU_GPIOA

#define RCU_GPIO_RS485_CTRL_PORT RCU_GPIOC

/*==================== Peripheral Clock ====================*/
#define RCU_GPIO_AFIO      RCU_AF

#define RCU_FPGA_SPI       RCU_SPI1
#define RCU_EEPROM_I2C     RCU_I2C0
#define RCU_MODBUS_USART   RCU_USART0
#define RCU_DMA0_FOR_UASRT RCU_DMA0

/*==================== Optional Peripheral Clock ====================*/

#define RCU_TIMER0_PWM RCU_TIMER0
#define RCU_TIMER1_PWM RCU_TIMER1
#define RCU_TIMER3_PWM RCU_TIMER3
#define RCU_TIMER4_PWM RCU_TIMER4
#define RCU_TIMER5_PWM RCU_TIMER5
#define RCU_TIMER6_PWM RCU_TIMER6

#define RCU_DAC_PERIPH RCU_DAC
#define RCU_SPI_EXTRA  RCU_SPI2

/*==================== Function ====================*/
void rcu_config(void);

#endif /* __RCU_H */
