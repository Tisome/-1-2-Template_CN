#include "rcu.h"

void rcu_config(void)
{
    /* AHB/APB 时钟分频配置 */
    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2);
    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);

    /*==================== GPIO Clock Enable ====================*/
    rcu_periph_clock_enable(RCU_GPIO_FPGA_INT_PORT);
    rcu_periph_clock_enable(RCU_GPIO_FPGA_START_PORT);

    rcu_periph_clock_enable(RCU_GPIO_FPGA_SPI_NSS_PORT);
    rcu_periph_clock_enable(RCU_GPIO_FPGA_SPI_SCK_PORT);
    rcu_periph_clock_enable(RCU_GPIO_FPGA_SPI_MISO_PORT);
    rcu_periph_clock_enable(RCU_GPIO_FPGA_SPI_MOSI_PORT);

    rcu_periph_clock_enable(RCU_GPIO_TFT_DC_PORT);
    rcu_periph_clock_enable(RCU_GPIO_TFT_CS_PORT);
    rcu_periph_clock_enable(RCU_GPIO_TFT_RESET_PORT);
    rcu_periph_clock_enable(RCU_GPIO_TFT_SCK_PORT);
    rcu_periph_clock_enable(RCU_GPIO_TFT_SDA_PORT);
    rcu_periph_clock_enable(RCU_GPIO_TFT_BL_PORT);

    rcu_periph_clock_enable(RCU_GPIO_KEY1_PORT);
    rcu_periph_clock_enable(RCU_GPIO_KEY2_PORT);
    rcu_periph_clock_enable(RCU_GPIO_KEY3_PORT);
    rcu_periph_clock_enable(RCU_GPIO_KEY4_PORT);

    rcu_periph_clock_enable(RCU_GPIO_EEPROM_I2C_PORT);

    rcu_periph_clock_enable(RCU_GPIO_USART0_PORT);

    rcu_periph_clock_enable(RCU_GPIO_RS485_CTRL_PORT);

    /*==================== AFIO Clock Enable ====================*/
    rcu_periph_clock_enable(RCU_GPIO_AFIO);

    /*==================== Peripheral Clock Enable ====================*/
    rcu_periph_clock_enable(RCU_FPGA_SPI);
    rcu_periph_clock_enable(RCU_TFT_SPI);
    rcu_periph_clock_enable(RCU_EEPROM_I2C);
    rcu_periph_clock_enable(RCU_MODBUS_USART);
    rcu_periph_clock_enable(RCU_DMA0_FOR_UASRT);

    /*==================== Optional Peripheral Clock Enable ====================*/
    /* 如果当前工程没用到，可以先注释掉，等后面真正用时再打开 */

    /* DMA */
    // rcu_periph_clock_enable(RCU_DMA_CH0);
    // rcu_periph_clock_enable(RCU_DMA_CH1);

    /* TIMER */
    // rcu_periph_clock_enable(RCU_TIMER0_PWM);
    // rcu_periph_clock_enable(RCU_TIMER1_PWM);
    // rcu_periph_clock_enable(RCU_TIMER3_PWM);
    // rcu_periph_clock_enable(RCU_TIMER4_PWM);
    // rcu_periph_clock_enable(RCU_TIMER5_PWM);
    // rcu_periph_clock_enable(RCU_TIMER6_PWM);

    /* DAC */
    // rcu_periph_clock_enable(RCU_DAC_PERIPH);

    /* Extra SPI */
    // rcu_periph_clock_enable(RCU_SPI_EXTRA);
}