#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "gd32f30x.h"
#include "user_type.h"

#define USE_HORIZONTAL 1

#define LCD_W 240
#define LCD_H 240

#define LCD_SCLK_GPIO_RCU_CLK RCU_GPIOC
#define LCD_SCLK_GPIO_PORT GPIOC
#define LCD_SCLK_GPIO_PIN GPIO_PIN_9

#define LCD_MOSI_GPIO_RCU_CLK RCU_GPIOC
#define LCD_MOSI_GPIO_PORT GPIOC
#define LCD_MOSI_GPIO_PIN GPIO_PIN_8

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

void LCD_GPIO_Init(void);
void LCD_Writ_Bus(u8 dat);
void LCD_WR_DATA8(u8 dat);
void LCD_WR_DATA(u16 dat);
void LCD_WR_REG(u8 dat);
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_Init(void);

#endif