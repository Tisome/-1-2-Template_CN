#include "gd32f30x.h"
#include "user_type.h"
#include "lcd.h"
#include "lcd_init.h"

void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)
{
    u16 i;
    u16 j;

    LCD_Address_Set(xsta, ysta, xend - 1U, yend - 1U);
    for (i = ysta; i < yend; i++)
    {
        for (j = xsta; j < xend; j++)
        {
            LCD_WR_DATA(color);
        }
    }
}

void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA(color);
}

void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    u16 t;
    int xerr = 0;
    int yerr = 0;
    int delta_x;
    int delta_y;
    int distance;
    int incx;
    int incy;
    int uRow;
    int uCol;

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0)
    {
        incx = 1;
    }
    else if (delta_x == 0)
    {
        incx = 0;
    }
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
    {
        incy = 1;
    }
    else if (delta_y == 0)
    {
        incy = 0;
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
    {
        distance = delta_x;
    }
    else
    {
        distance = delta_y;
    }

    for (t = 0; t < (u16)(distance + 1); t++)
    {
        LCD_DrawPoint((u16)uRow, (u16)uCol, color);
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

void Draw_Circle(u16 x0, u16 y0, u8 r, u16 color)
{
    int a = 0;
    int b = r;

    while (a <= b)
    {
        LCD_DrawPoint((u16)(x0 - b), (u16)(y0 - a), color);
        LCD_DrawPoint((u16)(x0 + b), (u16)(y0 - a), color);
        LCD_DrawPoint((u16)(x0 - a), (u16)(y0 + b), color);
        LCD_DrawPoint((u16)(x0 - a), (u16)(y0 - b), color);
        LCD_DrawPoint((u16)(x0 + b), (u16)(y0 + a), color);
        LCD_DrawPoint((u16)(x0 + a), (u16)(y0 - b), color);
        LCD_DrawPoint((u16)(x0 + a), (u16)(y0 + b), color);
        LCD_DrawPoint((u16)(x0 - b), (u16)(y0 + a), color);
        a++;
        if ((a * a + b * b) > (r * r))
        {
            b--;
        }
    }
}

void LCD_ShowPicture(u16 x, u16 y, u16 length, u16 width, const u8 pic[])
{
    u16 i;
    u16 j;
    u32 k = 0U;

    LCD_Address_Set(x, y, x + length - 1U, y + width - 1U);
    for (i = 0; i < length; i++)
    {
        for (j = 0; j < width; j++)
        {
            LCD_WR_DATA8(pic[k * 2U]);
            LCD_WR_DATA8(pic[k * 2U + 1U]);
            k++;
        }
    }
}