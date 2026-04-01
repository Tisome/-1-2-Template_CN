#include "gd32f30x.h"

#include <stddef.h>

#include "user_type.h"

#include "delay.h"
#include "lcd_init.h"

#define LCD_SPI_DMA_IRQ_PRIO 6U

static volatile bool s_lcd_dma_busy = false;
static volatile uint16_t s_lcd_frame_format = SPI_FRAMESIZE_8BIT;
static volatile lcd_dma_complete_cb_t s_lcd_dma_complete_cb = NULL;
static volatile void *s_lcd_dma_complete_user_data = NULL;
static uint16_t s_lcd_dma_dummy_word = 0U;

static void lcd_spi_wait_tbe(void)
{
    while (RESET == spi_i2s_flag_get(LCD_SPI_PERIPH, SPI_FLAG_TBE))
    {
    }
}

static void lcd_spi_wait_idle(void)
{
    lcd_spi_wait_tbe();
    while (SET == spi_i2s_flag_get(LCD_SPI_PERIPH, SPI_FLAG_TRANS))
    {
    }
}

static void lcd_dma_wait_idle(void)
{
    while (s_lcd_dma_busy)
    {
    }
}

static void lcd_spi_set_frame_format(uint16_t frame_format)
{
    if (s_lcd_frame_format == frame_format)
    {
        return;
    }

    lcd_spi_wait_idle();
    spi_disable(LCD_SPI_PERIPH);
    spi_i2s_data_frame_format_config(LCD_SPI_PERIPH, frame_format);
    spi_enable(LCD_SPI_PERIPH);

    s_lcd_frame_format = frame_format;
}

static void lcd_spi_write8_blocking(uint8_t dat)
{
    lcd_dma_wait_idle();
    lcd_spi_set_frame_format(SPI_FRAMESIZE_8BIT);
    lcd_spi_wait_tbe();
    spi_i2s_data_transmit(LCD_SPI_PERIPH, dat);
    lcd_spi_wait_idle();
}

static void lcd_spi_write16_blocking(uint16_t dat)
{
    lcd_dma_wait_idle();
    lcd_spi_set_frame_format(SPI_FRAMESIZE_16BIT);
    lcd_spi_wait_tbe();
    spi_i2s_data_transmit(LCD_SPI_PERIPH, dat);
    lcd_spi_wait_idle();
}

static void lcd_spi_dma_cleanup(void)
{
    dma_channel_disable(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL);
    spi_dma_disable(LCD_SPI_PERIPH, SPI_DMA_TRANSMIT);
    lcd_spi_wait_idle();
    LCD_CS_Set();
}

void LCD_GPIO_Init(void)
{
    dma_parameter_struct dma_init_struct;
    spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(LCD_SCLK_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_MOSI_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_RES_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_DC_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_CS_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_BLK_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_SPI_RCU_CLK);
    rcu_periph_clock_enable(LCD_SPI_DMA_RCU_CLK);

    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    gpio_init(LCD_SCLK_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, LCD_SCLK_GPIO_PIN);
    gpio_init(LCD_MOSI_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, LCD_MOSI_GPIO_PIN);
    gpio_init(LCD_RES_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_RES_GPIO_PIN);
    gpio_init(LCD_DC_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_DC_GPIO_PIN);
    gpio_init(LCD_CS_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_CS_GPIO_PIN);
    gpio_init(LCD_BLK_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_BLK_GPIO_PIN);

    LCD_RES_Set();
    LCD_DC_Set();
    LCD_CS_Set();
    LCD_BLK_Set();

    spi_i2s_deinit(LCD_SPI_PERIPH);
    spi_struct_para_init(&spi_init_struct);
    spi_init_struct.trans_mode = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_2;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init(LCD_SPI_PERIPH, &spi_init_struct);
    spi_enable(LCD_SPI_PERIPH);
    s_lcd_frame_format = SPI_FRAMESIZE_8BIT;

    dma_deinit(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)&s_lcd_dma_dummy_word;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_struct.number = 1U;
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(LCD_SPI_PERIPH);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, &dma_init_struct);
    dma_circulation_disable(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL);
    dma_memory_to_memory_disable(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL);
    dma_interrupt_flag_clear(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, DMA_INT_FLAG_G);
    dma_interrupt_enable(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, DMA_INT_FTF);
    dma_interrupt_enable(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, DMA_INT_ERR);

    nvic_irq_enable(LCD_SPI_DMA_IRQn, LCD_SPI_DMA_IRQ_PRIO, 0U);
}

void LCD_Writ_Bus(u8 dat)
{
    LCD_CS_Clr();
    lcd_spi_write8_blocking(dat);
    LCD_CS_Set();
}

void LCD_WR_DATA8(u8 dat)
{
    LCD_CS_Clr();
    lcd_spi_write8_blocking(dat);
    LCD_CS_Set();
}

void LCD_WR_DATA(u16 dat)
{
    LCD_CS_Clr();
    lcd_spi_write16_blocking(dat);
    LCD_CS_Set();
}

void LCD_WR_REG(u8 dat)
{
    LCD_DC_Clr();
    LCD_Writ_Bus(dat);
    LCD_DC_Set();
}

void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2)
{
    if (USE_HORIZONTAL == 0)
    {
        LCD_WR_REG(0x2A);
        LCD_WR_DATA(x1);
        LCD_WR_DATA(x2);
        LCD_WR_REG(0x2B);
        LCD_WR_DATA(y1);
        LCD_WR_DATA(y2);
        LCD_WR_REG(0x2C);
    }
    else if (USE_HORIZONTAL == 1)
    {
        LCD_WR_REG(0x2A);
        LCD_WR_DATA(x1);
        LCD_WR_DATA(x2);
        LCD_WR_REG(0x2B);
        LCD_WR_DATA(y1 + 80U);
        LCD_WR_DATA(y2 + 80U);
        LCD_WR_REG(0x2C);
    }
    else if (USE_HORIZONTAL == 2)
    {
        LCD_WR_REG(0x2A);
        LCD_WR_DATA(x1);
        LCD_WR_DATA(x2);
        LCD_WR_REG(0x2B);
        LCD_WR_DATA(y1);
        LCD_WR_DATA(y2);
        LCD_WR_REG(0x2C);
    }
    else
    {
        LCD_WR_REG(0x2A);
        LCD_WR_DATA(x1 + 80U);
        LCD_WR_DATA(x2 + 80U);
        LCD_WR_REG(0x2B);
        LCD_WR_DATA(y1);
        LCD_WR_DATA(y2);
        LCD_WR_REG(0x2C);
    }
}

void LCD_WriteColors_Blocking(const u16 *color_p, uint32_t pixel_count)
{
    if ((color_p == NULL) || (pixel_count == 0U))
    {
        return;
    }

    lcd_dma_wait_idle();
    lcd_spi_set_frame_format(SPI_FRAMESIZE_16BIT);
    LCD_CS_Clr();
    LCD_DC_Set();

    while (pixel_count > 0U)
    {
        lcd_spi_wait_tbe();
        spi_i2s_data_transmit(LCD_SPI_PERIPH, *color_p);
        color_p++;
        pixel_count--;
    }

    lcd_spi_wait_idle();
    LCD_CS_Set();
}

void LCD_WriteColorRepeated_Blocking(u16 color, uint32_t pixel_count)
{
    if (pixel_count == 0U)
    {
        return;
    }

    lcd_dma_wait_idle();
    lcd_spi_set_frame_format(SPI_FRAMESIZE_16BIT);
    LCD_CS_Clr();
    LCD_DC_Set();

    while (pixel_count > 0U)
    {
        lcd_spi_wait_tbe();
        spi_i2s_data_transmit(LCD_SPI_PERIPH, color);
        pixel_count--;
    }

    lcd_spi_wait_idle();
    LCD_CS_Set();
}

bool LCD_WriteColors_DMA(const u16 *color_p,
                         uint32_t pixel_count,
                         lcd_dma_complete_cb_t complete_cb,
                         void *user_data)
{
    if ((color_p == NULL) || (pixel_count == 0U) || (pixel_count > 65535U))
    {
        return false;
    }

    if (s_lcd_dma_busy)
    {
        return false;
    }

    lcd_spi_set_frame_format(SPI_FRAMESIZE_16BIT);

    dma_channel_disable(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL);
    spi_dma_disable(LCD_SPI_PERIPH, SPI_DMA_TRANSMIT);
    dma_interrupt_flag_clear(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, DMA_INT_FLAG_G);
    dma_memory_address_config(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, (uint32_t)color_p);
    dma_transfer_number_config(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, pixel_count);

    s_lcd_dma_complete_cb = complete_cb;
    s_lcd_dma_complete_user_data = user_data;
    s_lcd_dma_busy = true;

    LCD_CS_Clr();
    LCD_DC_Set();
    spi_dma_enable(LCD_SPI_PERIPH, SPI_DMA_TRANSMIT);
    dma_channel_enable(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL);

    return true;
}

bool LCD_DMA_IsBusy(void)
{
    return s_lcd_dma_busy;
}

void LCD_SPI2_DMA_IRQHandler(void)
{
    lcd_dma_complete_cb_t complete_cb;
    void *user_data;

    if ((SET != dma_interrupt_flag_get(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, DMA_INT_FLAG_FTF)) &&
        (SET != dma_interrupt_flag_get(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, DMA_INT_FLAG_ERR)))
    {
        return;
    }

    dma_interrupt_flag_clear(LCD_SPI_DMA_PERIPH, LCD_SPI_DMA_CHANNEL, DMA_INT_FLAG_G);
    lcd_spi_dma_cleanup();

    complete_cb = (lcd_dma_complete_cb_t)s_lcd_dma_complete_cb;
    user_data = (void *)s_lcd_dma_complete_user_data;
    s_lcd_dma_complete_cb = NULL;
    s_lcd_dma_complete_user_data = NULL;
    s_lcd_dma_busy = false;

    if (complete_cb != NULL)
    {
        complete_cb(user_data);
    }
}

void LCD_Init(void)
{
    LCD_GPIO_Init();

    LCD_RES_Clr();
    delay_ms(100U);
    LCD_RES_Set();
    delay_ms(100U);

    LCD_BLK_Set();
    delay_ms(100U);

    LCD_WR_REG(0x11);
    delay_ms(120U);

    LCD_WR_REG(0x36);
    if (USE_HORIZONTAL == 0)
    {
        LCD_WR_DATA8(0x00);
    }
    else if (USE_HORIZONTAL == 1)
    {
        LCD_WR_DATA8(0xC0);
    }
    else if (USE_HORIZONTAL == 2)
    {
        LCD_WR_DATA8(0x70);
    }
    else
    {
        LCD_WR_DATA8(0xA0);
    }

    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33);

    LCD_WR_REG(0xB7);
    LCD_WR_DATA8(0x35);

    LCD_WR_REG(0xBB);
    LCD_WR_DATA8(0x32);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x01);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x15);

    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x20);

    LCD_WR_REG(0xC6);
    LCD_WR_DATA8(0x0F);

    LCD_WR_REG(0xD0);
    LCD_WR_DATA8(0xA4);
    LCD_WR_DATA8(0xA1);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x31);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x48);
    LCD_WR_DATA8(0x17);
    LCD_WR_DATA8(0x14);
    LCD_WR_DATA8(0x15);
    LCD_WR_DATA8(0x31);
    LCD_WR_DATA8(0x34);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x15);
    LCD_WR_DATA8(0x31);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x48);
    LCD_WR_DATA8(0x17);
    LCD_WR_DATA8(0x14);
    LCD_WR_DATA8(0x15);
    LCD_WR_DATA8(0x31);
    LCD_WR_DATA8(0x34);
    LCD_WR_REG(0x21);

    LCD_WR_REG(0x29);
}
