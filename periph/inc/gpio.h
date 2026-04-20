#include "gd32f30x.h"

// FPGA INT GPIO
#define FPGA_INT_GPIO_PORT        GPIOA
#define FPGA_INT_GPIO_PIN         GPIO_PIN_11
#define FPGA_INT_GPIO_IRQN        EXTI10_15_IRQn
#define FPGA_INT_GPIO_PORT_SOURCE GPIO_PORT_SOURCE_GPIOA
#define FPGA_INT_GPIO_PIN_SOURCE  GPIO_PIN_SOURCE_11
#define FPGA_INT_EXTI             EXTI_11

#define FPGA_START_GPIO_PORT      GPIOA
#define FPGA_START_GPIO_PIN       GPIO_PIN_12

// FPGA SPI GPIO
#define FPGA_SPI                SPI1

#define FPGA_SPI_NSS_GPIO_PORT  GPIOB
#define FPGA_SPI_NSS_GPIO_PIN   GPIO_PIN_12

#define FPGA_SPI_SCK_GPIO_PORT  GPIOB
#define FPGA_SPI_SCK_GPIO_PIN   GPIO_PIN_13

#define FPGA_SPI_MISO_GPIO_PORT GPIOB
#define FPGA_SPI_MISO_GPIO_PIN  GPIO_PIN_14

#define FPGA_SPI_MOSI_GPIO_PORT GPIOB
#define FPGA_SPI_MOSI_GPIO_PIN  GPIO_PIN_15

#define FPGA_SPI_CS_DISABLE()                                        \
    do {                                                             \
        gpio_bit_set(FPGA_SPI_NSS_GPIO_PORT, FPGA_SPI_NSS_GPIO_PIN); \
    } while (0);

#define FPGA_SPI_CS_ENABLE()                                           \
    do {                                                               \
        gpio_bit_reset(FPGA_SPI_NSS_GPIO_PORT, FPGA_SPI_NSS_GPIO_PIN); \
    } while (0);

// KEY GPIO
#if CCT6 || CGT6
#define KEY1_GPIO_PORT        GPIOB
#define KEY1_GPIO_PIN         GPIO_PIN_0
#define KEY1_EXTI_IRQN        EXTI0_IRQn
#define KEY1_GPIO_PORT_SOURCE GPIO_PORT_SOURCE_GPIOB
#define KEY1_GPIO_PIN_SOURCE  GPIO_PIN_SOURCE_0
#define KEY1_EXTI             EXTI_0

#define KEY2_GPIO_PORT        GPIOA
#define KEY2_GPIO_PIN         GPIO_PIN_6
#define KEY2_EXTI_IRQN        EXTI5_9_IRQn
#define KEY2_GPIO_PORT_SOURCE GPIO_PORT_SOURCE_GPIOA
#define KEY2_GPIO_PIN_SOURCE  GPIO_PIN_SOURCE_6
#define KEY2_EXTI             EXTI_6

#define KEY3_GPIO_PORT        GPIOB
#define KEY3_GPIO_PIN         GPIO_PIN_1
#define KEY3_EXTI_IRQN        EXTI1_IRQn
#define KEY3_GPIO_PORT_SOURCE GPIO_PORT_SOURCE_GPIOB
#define KEY3_GPIO_PIN_SOURCE  GPIO_PIN_SOURCE_1
#define KEY3_EXTI             EXTI_1

#define KEY4_GPIO_PORT        GPIOB
#define KEY4_GPIO_PIN         GPIO_PIN_2
#define KEY4_EXTI_IRQN        EXTI2_IRQn
#define KEY4_GPIO_PORT_SOURCE GPIO_PORT_SOURCE_GPIOB
#define KEY4_GPIO_PIN_SOURCE  GPIO_PIN_SOURCE_2
#define KEY4_EXTI             EXTI_2

#elif RCT6
#define KEY1_GPIO_PORT        GPIOA
#define KEY1_GPIO_PIN         GPIO_PIN_1
#define KEY1_EXTI_IRQN        EXTI1_IRQn
#define KEY1_GPIO_PORT_SOURCE GPIO_PORT_SOURCE_GPIOA
#define KEY1_GPIO_PIN_SOURCE  GPIO_PIN_SOURCE_1
#define KEY1_EXTI             EXTI_1

#define KEY2_GPIO_PORT        GPIOA
#define KEY2_GPIO_PIN         GPIO_PIN_2
#define KEY2_EXTI_IRQN        EXTI2_IRQn
#define KEY2_GPIO_PORT_SOURCE GPIO_PORT_SOURCE_GPIOA
#define KEY2_GPIO_PIN_SOURCE  GPIO_PIN_SOURCE_2
#define KEY2_EXTI             EXTI_2

#define KEY3_GPIO_PORT        GPIOA
#define KEY3_GPIO_PIN         GPIO_PIN_3
#define KEY3_EXTI_IRQN        EXTI3_IRQn
#define KEY3_GPIO_PORT_SOURCE GPIO_PORT_SOURCE_GPIOA
#define KEY3_GPIO_PIN_SOURCE  GPIO_PIN_SOURCE_3
#define KEY3_EXTI             EXTI_3

#define KEY4_GPIO_PORT        GPIOA
#define KEY4_GPIO_PIN         GPIO_PIN_4
#define KEY4_EXTI_IRQN        EXTI4_IRQn
#define KEY4_GPIO_PORT_SOURCE GPIO_PORT_SOURCE_GPIOA
#define KEY4_GPIO_PIN_SOURCE  GPIO_PIN_SOURCE_4
#define KEY4_EXTI             EXTI_4
#endif

// EEPROM IIC GPIO
#define EEPROM_I2C           I2C0
#define EEPROM_I2C_GPIO_PORT GPIOB
#define EEPROM_I2C_SCL_PIN   GPIO_PIN_6
#define EEPROM_I2C_SDA_PIN   GPIO_PIN_7

// MODBUS USART0 GPIO
#define USART0_GPIO_PORT   GPIOA
#define USART0_TX_GPIO_PIN GPIO_PIN_9
#define USART0_RX_GPIO_PIN GPIO_PIN_10

// Rs485 control
#define RS485_CONTROL_PIN       GPIO_PIN_14
#define RS485_CONTROL_GPIO_PORT GPIOC
void gpio_config();
