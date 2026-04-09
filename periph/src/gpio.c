#include "gd32f30x.h"

#include "gpio.h"
#include "app_config.h"

void gpio_config()
{
    /* disable JTAG, enable SWD */
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    /* disable JTAG, enable SWD */

    /* configure FPGA INT GPIO */
#if ENABLE_FPGA_SPI_COMM_TEST
    gpio_init(FPGA_INT_GPIO_PORT, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, FPGA_INT_GPIO_PIN);
#else
    gpio_init(FPGA_INT_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, FPGA_INT_GPIO_PIN);
#endif

    nvic_irq_enable(FPGA_INT_GPIO_IRQN, 5U, 0U);

    gpio_exti_source_select(FPGA_INT_GPIO_PORT_SOURCE, FPGA_INT_GPIO_PIN_SOURCE);

#if ENABLE_FPGA_SPI_COMM_TEST
    exti_init(FPGA_INT_EXTI, EXTI_INTERRUPT, EXTI_TRIG_RISING);
#else
    exti_init(FPGA_INT_EXTI, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
#endif

    exti_interrupt_flag_clear(FPGA_INT_EXTI);
    /* configure FPGA INT GPIO */

    /* configure FPGA START GPIO */
    gpio_init(FPGA_START_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FPGA_START_GPIO_PIN);
    gpio_bit_reset(FPGA_START_GPIO_PORT, FPGA_START_GPIO_PIN);
    /* configure FPGA START GPIO */

    /* configure FPGA SPI GPIO */
    gpio_init(FPGA_SPI_NSS_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FPGA_SPI_NSS_GPIO_PIN);
    gpio_init(FPGA_SPI_SCK_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, FPGA_SPI_SCK_GPIO_PIN);
    gpio_init(FPGA_SPI_MOSI_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, FPGA_SPI_MOSI_GPIO_PIN);
    gpio_init(FPGA_SPI_MISO_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, FPGA_SPI_MISO_GPIO_PIN);

    /* disable spi */
    FPGA_SPI_CS_DISABLE();
    /* configure FPGA SPI GPIO */

#if CCT6
    /* configure TFT LCD GPIO */
    gpio_init(TFT_DC_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, TFT_DC_GPIO_PIN);
    gpio_init(TFT_RESET_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, TFT_RESET_GPIO_PIN);
    gpio_init(TFT_BL_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, TFT_BL_GPIO_PIN);

    gpio_init(TFT_SCK_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, TFT_SCK_GPIO_PIN);
    gpio_init(TFT_SDA_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, TFT_SDA_GPIO_PIN);
    gpio_init(TFT_CS_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, TFT_CS_GPIO_PIN);
/* configure TFT LCD GPIO */
#endif

    /* configure KEY GPIO port */
    gpio_init(KEY1_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY1_GPIO_PIN);
    gpio_init(KEY2_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY2_GPIO_PIN);
    gpio_init(KEY3_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY3_GPIO_PIN);
    gpio_init(KEY4_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY4_GPIO_PIN);

    nvic_irq_enable(KEY1_EXTI_IRQN, 5U, 0U);
    nvic_irq_enable(KEY2_EXTI_IRQN, 5U, 0U);
    nvic_irq_enable(KEY3_EXTI_IRQN, 5U, 0U);
    nvic_irq_enable(KEY4_EXTI_IRQN, 5U, 0U);

    gpio_exti_source_select(KEY1_GPIO_PORT_SOURCE, KEY1_GPIO_PIN_SOURCE);
    gpio_exti_source_select(KEY2_GPIO_PORT_SOURCE, KEY2_GPIO_PIN_SOURCE);
    gpio_exti_source_select(KEY3_GPIO_PORT_SOURCE, KEY3_GPIO_PIN_SOURCE);
    gpio_exti_source_select(KEY4_GPIO_PORT_SOURCE, KEY4_GPIO_PIN_SOURCE);

    exti_init(KEY1_EXTI, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_init(KEY2_EXTI, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_init(KEY3_EXTI, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_init(KEY4_EXTI, EXTI_INTERRUPT, EXTI_TRIG_BOTH);

    exti_interrupt_flag_clear(KEY1_EXTI);
    exti_interrupt_flag_clear(KEY2_EXTI);
    exti_interrupt_flag_clear(KEY3_EXTI);
    exti_interrupt_flag_clear(KEY4_EXTI);
    /* configure KEY GPIO port */

    /* configure E2PROM GPIO port */
    gpio_init(EEPROM_I2C_GPIO_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, EEPROM_I2C_SDA_PIN);
    gpio_init(EEPROM_I2C_GPIO_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, EEPROM_I2C_SCL_PIN);

    gpio_bit_set(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SDA_PIN);
    gpio_bit_set(EEPROM_I2C_GPIO_PORT, EEPROM_I2C_SCL_PIN);
    /* configure E2PROM GPIO port */

    /* configure MODBUS USART0 GPIO port */
    gpio_init(USART0_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, USART0_TX_GPIO_PIN);
    gpio_init(USART0_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, USART0_RX_GPIO_PIN);

#if CCT6
    /* configure RS485 Control GPIO port (CCT6 only) */
    gpio_init(RS485_CONTROL_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RS485_CONTROL_PIN);
    gpio_bit_reset(RS485_CONTROL_GPIO_PORT, RS485_CONTROL_PIN);
#endif
}

void fpga_spi_cs_high(void)
{
    gpio_bit_set(FPGA_SPI_NSS_GPIO_PORT, FPGA_SPI_NSS_GPIO_PIN);
}
