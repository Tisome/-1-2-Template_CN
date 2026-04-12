#include "st7789.h"

#include <stddef.h>

#include "delay.h"

#define ST7789_SPI_DMA_IRQ_PRIO 6U

#define ST7789_CMD_SLPOUT 0x11
#define ST7789_CMD_INVON  0x21
#define ST7789_CMD_DISPON 0x29
#define ST7789_CMD_CASET  0x2A
#define ST7789_CMD_RASET  0x2B
#define ST7789_CMD_RAMWR  0x2C
#define ST7789_CMD_MADCTL 0x36
#define ST7789_CMD_COLMOD 0x3A
#define ST7789_CMD_NORON  0x13

#define ST7789_COLMOD_RGB565 0x55U
#define ST7789_MADCTL_RGB    0x00U

static volatile bool s_st7789_dma_busy = false;
static volatile uint16_t s_st7789_frame_format = SPI_FRAMESIZE_8BIT;
static volatile st7789_dma_complete_cb_t s_st7789_dma_complete_cb = NULL;
static volatile void *s_st7789_dma_complete_user_data = NULL;

static void st7789_spi_wait_tbe(void)
{
    while (RESET == spi_i2s_flag_get(ST7789_SPI_PERIPH, SPI_FLAG_TBE))
    {
    }
}

static void st7789_spi_wait_idle(void)
{
    st7789_spi_wait_tbe();
    while (SET == spi_i2s_flag_get(ST7789_SPI_PERIPH, SPI_FLAG_TRANS))
    {
    }
}

static void st7789_dma_wait_idle(void)
{
    while (s_st7789_dma_busy)
    {
    }
}

static void st7789_spi_set_frame_format(uint16_t frame_format)
{
    if (s_st7789_frame_format == frame_format)
    {
        return;
    }

    st7789_spi_wait_idle();
    spi_disable(ST7789_SPI_PERIPH);
    spi_i2s_data_frame_format_config(ST7789_SPI_PERIPH, frame_format);
    spi_enable(ST7789_SPI_PERIPH);
    s_st7789_frame_format = frame_format;
}

static void st7789_spi_write8_blocking(uint8_t dat)
{
    st7789_dma_wait_idle();
    st7789_spi_set_frame_format(SPI_FRAMESIZE_8BIT);
    st7789_spi_wait_tbe();
    spi_i2s_data_transmit(ST7789_SPI_PERIPH, dat);
    st7789_spi_wait_idle();
}

static void st7789_spi_write16_blocking(uint16_t dat)
{
    st7789_dma_wait_idle();
    st7789_spi_set_frame_format(SPI_FRAMESIZE_16BIT);
    st7789_spi_wait_tbe();
    spi_i2s_data_transmit(ST7789_SPI_PERIPH, dat);
    st7789_spi_wait_idle();
}

static void st7789_write_reg(uint8_t reg)
{
    ST7789_DC_Clr();
    ST7789_CS_Clr();
    st7789_spi_write8_blocking(reg);
    ST7789_CS_Set();
    ST7789_DC_Set();
}

static void st7789_write_data8(uint8_t dat)
{
    ST7789_DC_Set();
    ST7789_CS_Clr();
    st7789_spi_write8_blocking(dat);
    ST7789_CS_Set();
}

static void st7789_write_data16(uint16_t dat)
{
    ST7789_DC_Set();
    ST7789_CS_Clr();
    st7789_spi_write16_blocking(dat);
    ST7789_CS_Set();
}

static void st7789_write_repeated_color_blocking(uint16_t color, uint32_t pixel_count)
{
    if (pixel_count == 0U)
    {
        return;
    }

    st7789_dma_wait_idle();
    st7789_spi_set_frame_format(SPI_FRAMESIZE_16BIT);
    ST7789_CS_Clr();
    ST7789_DC_Set();

    while (pixel_count > 0U)
    {
        st7789_spi_wait_tbe();
        spi_i2s_data_transmit(ST7789_SPI_PERIPH, color);
        pixel_count--;
    }

    st7789_spi_wait_idle();
    ST7789_CS_Set();
}

static void st7789_spi_dma_cleanup(void)
{
    dma_channel_disable(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL);
    spi_dma_disable(ST7789_SPI_PERIPH, SPI_DMA_TRANSMIT);
    st7789_spi_wait_idle();
    ST7789_CS_Set();
}

static void st7789_send_gamma_sequence(uint8_t reg, const uint8_t *data, uint32_t len)
{
    uint32_t i;

    st7789_write_reg(reg);
    for (i = 0U; i < len; i++)
    {
        st7789_write_data8(data[i]);
    }
}

static void st7789_gpio_init(void)
{
    dma_parameter_struct dma_init_struct;
    spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(ST7789_SCLK_GPIO_RCU_CLK);
    rcu_periph_clock_enable(ST7789_MOSI_GPIO_RCU_CLK);
    rcu_periph_clock_enable(ST7789_DC_GPIO_RCU_CLK);
    rcu_periph_clock_enable(ST7789_CS_GPIO_RCU_CLK);
    rcu_periph_clock_enable(ST7789_RES_GPIO_RCU_CLK);
    rcu_periph_clock_enable(ST7789_BL_GPIO_RCU_CLK);
    rcu_periph_clock_enable(ST7789_SPI_RCU_CLK);
    rcu_periph_clock_enable(ST7789_SPI_DMA_RCU_CLK);

    /* Free PB3/PB4/PA15 from JTAG while keeping SWD available. */
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    gpio_init(ST7789_SCLK_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, ST7789_SCLK_GPIO_PIN);
    gpio_init(ST7789_MOSI_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, ST7789_MOSI_GPIO_PIN);
    gpio_init(ST7789_DC_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ST7789_DC_GPIO_PIN);
    gpio_init(ST7789_CS_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ST7789_CS_GPIO_PIN);
    gpio_init(ST7789_RES_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ST7789_RES_GPIO_PIN);
    gpio_init(ST7789_BL_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, ST7789_BL_GPIO_PIN);

    ST7789_RES_Set();
    ST7789_DC_Set();
    ST7789_CS_Set();
    ST7789_BL_Set();

    spi_i2s_deinit(ST7789_SPI_PERIPH);
    spi_struct_para_init(&spi_init_struct);
    spi_init_struct.trans_mode = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_2;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init(ST7789_SPI_PERIPH, &spi_init_struct);
    spi_enable(ST7789_SPI_PERIPH);
    s_st7789_frame_format = SPI_FRAMESIZE_8BIT;

    dma_deinit(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = 0U;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_struct.number = 1U;
    dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(ST7789_SPI_PERIPH);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, &dma_init_struct);
    dma_circulation_disable(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL);
    dma_memory_to_memory_disable(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL);
    dma_interrupt_flag_clear(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, DMA_INT_FLAG_G);
    dma_interrupt_enable(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, DMA_INT_FTF);
    dma_interrupt_enable(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, DMA_INT_ERR);

    nvic_irq_enable(ST7789_SPI_DMA_IRQn, ST7789_SPI_DMA_IRQ_PRIO, 0U);
}

void ST7789_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    st7789_write_reg(ST7789_CMD_CASET);
    st7789_write_data16(x1);
    st7789_write_data16(x2);

    st7789_write_reg(ST7789_CMD_RASET);
    st7789_write_data16(y1);
    st7789_write_data16(y2);

    st7789_write_reg(ST7789_CMD_RAMWR);
}

void ST7789_WriteColors_Blocking(const uint16_t *color_p, uint32_t pixel_count)
{
    if ((color_p == NULL) || (pixel_count == 0U))
    {
        return;
    }

    st7789_dma_wait_idle();
    st7789_spi_set_frame_format(SPI_FRAMESIZE_16BIT);
    ST7789_CS_Clr();
    ST7789_DC_Set();

    while (pixel_count > 0U)
    {
        st7789_spi_wait_tbe();
        spi_i2s_data_transmit(ST7789_SPI_PERIPH, *color_p);
        color_p++;
        pixel_count--;
    }

    st7789_spi_wait_idle();
    ST7789_CS_Set();
}

bool ST7789_WriteColors_DMA(const uint16_t *color_p,
                            uint32_t pixel_count,
                            st7789_dma_complete_cb_t complete_cb,
                            void *user_data)
{
    if ((color_p == NULL) || (pixel_count == 0U) || (pixel_count > 65535U))
    {
        return false;
    }

    if (s_st7789_dma_busy)
    {
        return false;
    }

    st7789_spi_set_frame_format(SPI_FRAMESIZE_16BIT);

    dma_channel_disable(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL);
    spi_dma_disable(ST7789_SPI_PERIPH, SPI_DMA_TRANSMIT);
    dma_interrupt_flag_clear(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, DMA_INT_FLAG_G);
    dma_memory_address_config(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, (uint32_t)color_p);
    dma_transfer_number_config(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, pixel_count);

    s_st7789_dma_complete_cb = complete_cb;
    s_st7789_dma_complete_user_data = user_data;
    s_st7789_dma_busy = true;

    ST7789_CS_Clr();
    ST7789_DC_Set();
    spi_dma_enable(ST7789_SPI_PERIPH, SPI_DMA_TRANSMIT);
    dma_channel_enable(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL);

    return true;
}

bool ST7789_DMA_IsBusy(void)
{
    return s_st7789_dma_busy;
}

void ST7789_SPI2_DMA_IRQHandler(void)
{
    st7789_dma_complete_cb_t complete_cb;
    void *user_data;

    if ((SET != dma_interrupt_flag_get(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, DMA_INT_FLAG_FTF)) &&
        (SET != dma_interrupt_flag_get(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, DMA_INT_FLAG_ERR)))
    {
        return;
    }

    dma_interrupt_flag_clear(ST7789_SPI_DMA_PERIPH, ST7789_SPI_DMA_CHANNEL, DMA_INT_FLAG_G);
    st7789_spi_dma_cleanup();

    complete_cb = (st7789_dma_complete_cb_t)s_st7789_dma_complete_cb;
    user_data = (void *)s_st7789_dma_complete_user_data;
    s_st7789_dma_complete_cb = NULL;
    s_st7789_dma_complete_user_data = NULL;
    s_st7789_dma_busy = false;

    if (complete_cb != NULL)
    {
        complete_cb(user_data);
    }
}

void ST7789_Init(void)
{
    static const uint8_t gamma_pos[] = {
        0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
    static const uint8_t gamma_neg[] = {
        0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};

    s_st7789_dma_busy = false;
    s_st7789_dma_complete_cb = NULL;
    s_st7789_dma_complete_user_data = NULL;
    st7789_gpio_init();

    ST7789_RES_Clr();
    delay_ms(10U);
    ST7789_RES_Set();
    delay_ms(20U);

    st7789_write_reg(ST7789_CMD_COLMOD);
    st7789_write_data8(ST7789_COLMOD_RGB565);

    st7789_write_reg(0xB2);
    st7789_write_data8(0x0C);
    st7789_write_data8(0x0C);
    st7789_write_data8(0x00);
    st7789_write_data8(0x33);
    st7789_write_data8(0x33);

    st7789_write_reg(ST7789_CMD_MADCTL);
    st7789_write_data8(ST7789_MADCTL_RGB);

    st7789_write_reg(0xB7);
    st7789_write_data8(0x35);

    st7789_write_reg(0xBB);
    st7789_write_data8(0x19);

    st7789_write_reg(0xC0);
    st7789_write_data8(0x2C);

    st7789_write_reg(0xC2);
    st7789_write_data8(0x01);

    st7789_write_reg(0xC3);
    st7789_write_data8(0x12);

    st7789_write_reg(0xC4);
    st7789_write_data8(0x20);

    st7789_write_reg(0xC6);
    st7789_write_data8(0x0F);

    st7789_write_reg(0xD0);
    st7789_write_data8(0xA4);
    st7789_write_data8(0xA1);

    st7789_send_gamma_sequence(0xE0, gamma_pos, sizeof(gamma_pos));
    st7789_send_gamma_sequence(0xE1, gamma_neg, sizeof(gamma_neg));

    st7789_write_reg(ST7789_CMD_INVON);
    st7789_write_reg(ST7789_CMD_SLPOUT);
    delay_ms(120U);
    st7789_write_reg(ST7789_CMD_NORON);
    st7789_write_reg(ST7789_CMD_DISPON);
    delay_ms(50U);

    ST7789_BL_Set();
    ST7789_Fill(0U, 0U, ST7789_W, ST7789_H, BLACK);
}

void ST7789_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
    uint32_t pixel_count;

    if ((xend <= xsta) || (yend <= ysta))
    {
        return;
    }

    ST7789_Address_Set(xsta, ysta, (uint16_t)(xend - 1U), (uint16_t)(yend - 1U));
    pixel_count = (uint32_t)(xend - xsta) * (uint32_t)(yend - ysta);
    st7789_write_repeated_color_blocking(color, pixel_count);
}
