#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "gd32f30x.h"
#include "user_type.h"

#define USE_HORIZONTAL 1 // 设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#define LCD_W 240
#define LCD_H 240

/* LCD SPI 引脚定义 */

// 0/1/2/3/4/5/6/7
// SCL
#define LCD_SCLK_GPIO_RCU_CLK RCU_GPIOC
#define LCD_SCLK_GPIO_PORT GPIOC
#define LCD_SCLK_GPIO_PIN GPIO_PIN_9

// SDA
#define LCD_MOSI_GPIO_RCU_CLK RCU_GPIOC
#define LCD_MOSI_GPIO_PORT GPIOC
#define LCD_MOSI_GPIO_PIN GPIO_PIN_8

// RES
#define LCD_RES_GPIO_RCU_CLK RCU_GPIOC
#define LCD_RES_GPIO_PORT GPIOC
#define LCD_RES_GPIO_PIN GPIO_PIN_6

// DC
#define LCD_DC_GPIO_RCU_CLK RCU_GPIOC
#define LCD_DC_GPIO_PORT GPIOC
#define LCD_DC_GPIO_PIN GPIO_PIN_7

// CS1
#define LCD_CS_GPIO_RCU_CLK RCU_GPIOB
#define LCD_CS_GPIO_PORT GPIOB
#define LCD_CS_GPIO_PIN GPIO_PIN_14

// BLK
#define LCD_BLK_GPIO_RCU_CLK RCU_GPIOB
#define LCD_BLK_GPIO_PORT GPIOB
#define LCD_BLK_GPIO_PIN GPIO_PIN_15

/* 字库 SPI */
// FS0
#define ZK_MISO_GPIO_RCU_CLK RCU_GPIOB
#define ZK_MISO_GPIO_PORT GPIOB
#define ZK_MISO_GPIO_PIN GPIO_PIN_12

// CS2
#define ZK_CS_GPIO_RCU_CLK RCU_GPIOB
#define ZK_CS_GPIO_PORT GPIOB
#define ZK_CS_GPIO_PIN GPIO_PIN_13

//-----------------LCD端口定义----------------

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

#define ZK_MISO gpio_input_bit_get(ZK_MISO_GPIO_PORT, ZK_MISO_GPIO_PIN)

#define ZK_CS_Clr() gpio_bit_reset(ZK_CS_GPIO_PORT, ZK_CS_GPIO_PIN)
#define ZK_CS_Set() gpio_bit_set(ZK_CS_GPIO_PORT, ZK_CS_GPIO_PIN)

void LCD_GPIO_Init(void);                             // 初始化GPIO
void LCD_Writ_Bus(u8 dat);                            // 模拟SPI时序
void LCD_WR_DATA8(u8 dat);                            // 写入一个字节
void LCD_WR_DATA(u16 dat);                            // 写入两个字节
void LCD_WR_REG(u8 dat);                              // 写入一个指令
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2); // 设置坐标函数
void LCD_Init(void);                                  // LCD初始化
#endif
