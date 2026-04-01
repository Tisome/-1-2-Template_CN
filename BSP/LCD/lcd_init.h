#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include <stdbool.h>
#include <stdint.h>

#include "gd32f30x.h"
#include "user_type.h"

#define USE_HORIZONTAL 0

#define LCD_W 240
#define LCD_H 240

#define LCD_SCLK_GPIO_RCU_CLK RCU_GPIOB
#define LCD_SCLK_GPIO_PORT GPIOB
#define LCD_SCLK_GPIO_PIN GPIO_PIN_3

#define LCD_MOSI_GPIO_RCU_CLK RCU_GPIOB
#define LCD_MOSI_GPIO_PORT GPIOB
#define LCD_MOSI_GPIO_PIN GPIO_PIN_5

#define LCD_RES_GPIO_RCU_CLK RCU_GPIOC
#define LCD_RES_GPIO_PORT GPIOC
#define LCD_RES_GPIO_PIN GPIO_PIN_6

#define LCD_DC_GPIO_RCU_CLK RCU_GPIOC
#define LCD_DC_GPIO_PORT GPIOC
#define LCD_DC_GPIO_PIN GPIO_PIN_7

#define LCD_CS_GPIO_RCU_CLK RCU_GPIOB
#define LCD_CS_GPIO_PORT GPIOB
#define LCD_CS_GPIO_PIN GPIO_PIN_14

#define LCD_BLK_GPIO_RCU_CLK RCU_GPIOB
#define LCD_BLK_GPIO_PORT GPIOB
#define LCD_BLK_GPIO_PIN GPIO_PIN_15

#define LCD_SPI_PERIPH SPI2
#define LCD_SPI_RCU_CLK RCU_SPI2
#define LCD_SPI_DMA_PERIPH DMA1
#define LCD_SPI_DMA_RCU_CLK RCU_DMA1
#define LCD_SPI_DMA_CHANNEL DMA_CH1
#define LCD_SPI_DMA_IRQn DMA1_Channel1_IRQn

#define LCD_SCLK_Clr() gpio_bit_reset(LCD_SCLK_GPIO_PORT, LCD_SCLK_GPIO_PIN)
#define LCD_SCLK_Set() gpio_bit_set(LCD_SCLK_GPIO_PORT, LCD_SCLK_GPIO_PIN)

#define LCD_MOSI_Clr() gpio_bit_reset(LCD_MOSI_GPIO_PORT, LCD_MOSI_GPIO_PIN)
#define LCD_MOSI_Set() gpio_bit_set(LCD_MOSI_GPIO_PORT, LCD_MOSI_GPIO_PIN)

#define LCD_RES_Clr() gpio_bit_reset(LCD_RES_GPIO_PORT, LCD_RES_GPIO_PIN)
#define LCD_RES_Set() gpio_bit_set(LCD_RES_GPIO_PORT, LCD_RES_GPIO_PIN)

#define LCD_DC_Clr() gpio_bit_reset(LCD_DC_GPIO_PORT, LCD_DC_GPIO_PIN)
#define LCD_DC_Set() gpio_bit_set(LCD_DC_GPIO_PORT, LCD_DC_GPIO_PIN)

#define LCD_CS_Clr() gpio_bit_reset(LCD_CS_GPIO_PORT, LCD_CS_GPIO_PIN)
#define LCD_CS_Set() gpio_bit_set(LCD_CS_GPIO_PORT, LCD_CS_GPIO_PIN)

#define LCD_BLK_Clr() gpio_bit_reset(LCD_BLK_GPIO_PORT, LCD_BLK_GPIO_PIN)
#define LCD_BLK_Set() gpio_bit_set(LCD_BLK_GPIO_PORT, LCD_BLK_GPIO_PIN)

typedef void (*lcd_dma_complete_cb_t)(void *user_data);

void LCD_GPIO_Init(void);
void LCD_Writ_Bus(u8 dat);
void LCD_WR_DATA8(u8 dat);
void LCD_WR_DATA(u16 dat);
void LCD_WR_REG(u8 dat);
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_WriteColors_Blocking(const u16 *color_p, uint32_t pixel_count);
void LCD_WriteColorRepeated_Blocking(u16 color, uint32_t pixel_count);
bool LCD_WriteColors_DMA(const u16 *color_p,
                         uint32_t pixel_count,
                         lcd_dma_complete_cb_t complete_cb,
                         void *user_data);
bool LCD_DMA_IsBusy(void);
void LCD_SPI2_DMA_IRQHandler(void);
void LCD_Init(void);

#endif
