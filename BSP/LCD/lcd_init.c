#include "gd32f30x.h"

#include "user_type.h"

#include "delay.h"
#include "lcd_init.h"

void LCD_GPIO_Init(void)
{
    rcu_periph_clock_enable(LCD_SCLK_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_MOSI_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_RES_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_DC_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_CS_GPIO_RCU_CLK);
    rcu_periph_clock_enable(LCD_BLK_GPIO_RCU_CLK);

    gpio_init(LCD_SCLK_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_SCLK_GPIO_PIN);
    gpio_init(LCD_MOSI_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_MOSI_GPIO_PIN);
    gpio_init(LCD_RES_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_RES_GPIO_PIN);
    gpio_init(LCD_DC_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_DC_GPIO_PIN);
    gpio_init(LCD_CS_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_CS_GPIO_PIN);
    gpio_init(LCD_BLK_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCD_BLK_GPIO_PIN);

    gpio_bit_set(LCD_SCLK_GPIO_PORT, LCD_SCLK_GPIO_PIN);
    gpio_bit_set(LCD_MOSI_GPIO_PORT, LCD_MOSI_GPIO_PIN);
    gpio_bit_set(LCD_RES_GPIO_PORT, LCD_RES_GPIO_PIN);
    gpio_bit_set(LCD_DC_GPIO_PORT, LCD_DC_GPIO_PIN);
    gpio_bit_set(LCD_CS_GPIO_PORT, LCD_CS_GPIO_PIN);
    gpio_bit_set(LCD_BLK_GPIO_PORT, LCD_BLK_GPIO_PIN);
}

void LCD_Writ_Bus(u8 dat)
{
    u8 i;

    LCD_CS_Clr();
    for (i = 0; i < 8; i++)
    {
        LCD_SCLK_Clr();
        if (dat & 0x80)
        {
            LCD_MOSI_Set();
        }
        else
        {
            LCD_MOSI_Clr();
        }
        LCD_SCLK_Set();
        dat <<= 1;
    }
    LCD_CS_Set();
}

void LCD_WR_DATA8(u8 dat)
{
    LCD_Writ_Bus(dat);
}

void LCD_WR_DATA(u16 dat)
{
    LCD_Writ_Bus((u8)(dat >> 8));
    LCD_Writ_Bus((u8)dat);
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