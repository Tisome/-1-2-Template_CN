#ifndef __ST7789_H
#define __ST7789_H

#include <stdbool.h>
#include <stdint.h>

#include "gd32f30x.h"

#define ST7789_W 240
#define ST7789_H 240

#define ST7789_DC_GPIO_RCU_CLK    RCU_GPIOB
#define ST7789_DC_GPIO_PORT       GPIOB
#define ST7789_DC_GPIO_PIN        GPIO_PIN_4

#define ST7789_CS_GPIO_RCU_CLK    RCU_GPIOA
#define ST7789_CS_GPIO_PORT       GPIOA
#define ST7789_CS_GPIO_PIN        GPIO_PIN_15

#define ST7789_RES_GPIO_RCU_CLK   RCU_GPIOC
#define ST7789_RES_GPIO_PORT      GPIOC
#define ST7789_RES_GPIO_PIN       GPIO_PIN_13

#define ST7789_SCLK_GPIO_RCU_CLK  RCU_GPIOB
#define ST7789_SCLK_GPIO_PORT     GPIOB
#define ST7789_SCLK_GPIO_PIN      GPIO_PIN_3

#define ST7789_MOSI_GPIO_RCU_CLK  RCU_GPIOB
#define ST7789_MOSI_GPIO_PORT     GPIOB
#define ST7789_MOSI_GPIO_PIN      GPIO_PIN_5

#define ST7789_BL_GPIO_RCU_CLK    RCU_GPIOB
#define ST7789_BL_GPIO_PORT       GPIOB
#define ST7789_BL_GPIO_PIN        GPIO_PIN_9

#define ST7789_SPI_PERIPH         SPI2
#define ST7789_SPI_RCU_CLK        RCU_SPI2
#define ST7789_SPI_DMA_PERIPH     DMA1
#define ST7789_SPI_DMA_RCU_CLK    RCU_DMA1
#define ST7789_SPI_DMA_CHANNEL    DMA_CH1
#define ST7789_SPI_DMA_IRQn       DMA1_Channel1_IRQn

#define ST7789_RES_Clr() gpio_bit_reset(ST7789_RES_GPIO_PORT, ST7789_RES_GPIO_PIN)
#define ST7789_RES_Set() gpio_bit_set(ST7789_RES_GPIO_PORT, ST7789_RES_GPIO_PIN)

#define ST7789_DC_Clr()  gpio_bit_reset(ST7789_DC_GPIO_PORT, ST7789_DC_GPIO_PIN)
#define ST7789_DC_Set()  gpio_bit_set(ST7789_DC_GPIO_PORT, ST7789_DC_GPIO_PIN)

#define ST7789_CS_Clr()  gpio_bit_reset(ST7789_CS_GPIO_PORT, ST7789_CS_GPIO_PIN)
#define ST7789_CS_Set()  gpio_bit_set(ST7789_CS_GPIO_PORT, ST7789_CS_GPIO_PIN)

#define ST7789_BL_Clr()  gpio_bit_reset(ST7789_BL_GPIO_PORT, ST7789_BL_GPIO_PIN)
#define ST7789_BL_Set()  gpio_bit_set(ST7789_BL_GPIO_PORT, ST7789_BL_GPIO_PIN)

typedef void (*st7789_dma_complete_cb_t)(void *user_data);

void ST7789_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7789_WriteColors_Blocking(const uint16_t *color_p, uint32_t pixel_count);
bool ST7789_WriteColors_DMA(const uint16_t *color_p,
                            uint32_t pixel_count,
                            st7789_dma_complete_cb_t complete_cb,
                            void *user_data);
bool ST7789_DMA_IsBusy(void);
void ST7789_SPI2_DMA_IRQHandler(void);
void ST7789_Init(void);
void ST7789_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);

#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define GRAY        0x8430
#define BRED        0xF81F
#define GRED        0xFFE0
#define GBLUE       0x07FF
#define BROWN       0xBC40
#define BRRED       0xFC07
#define DARKBLUE    0x01CF
#define LIGHTBLUE   0x7D7C
#define GRAYBLUE    0x5458
#define LIGHTGREEN  0x841F
#define LGRAY       0xE73C
#define LGRAYBLUE   0xA651
#define LBBLUE      0x2B12

#endif
