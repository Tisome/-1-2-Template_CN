/**
 * @file lv_port_disp_templ.c
 */

#if 1

#include "lv_port_disp_template.h"
#include "../../lvgl.h"
#include "lcd.h"
#include "lcd_init.h"

#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 240
#define DISP_BUF_LINES 3

static void disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
static void disp_write_colors(const lv_color_t *color_p, uint32_t pixel_count);
static void disp_flush_complete(void *user_data);

void lv_port_disp_init(void)
{
    static lv_disp_draw_buf_t draw_buf_dsc;
    static lv_color_t buf_1[MY_DISP_HOR_RES * DISP_BUF_LINES];
    static lv_disp_drv_t disp_drv;

    disp_init();

    lv_disp_draw_buf_init(&draw_buf_dsc, buf_1, NULL, MY_DISP_HOR_RES * DISP_BUF_LINES);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf_dsc;

    lv_disp_drv_register(&disp_drv);
}

static void disp_init(void)
{
    LCD_Init();
    LCD_Fill(0U, 0U, MY_DISP_HOR_RES, MY_DISP_VER_RES, BLACK);
}

static void disp_write_colors(const lv_color_t *color_p, uint32_t pixel_count)
{
    LCD_WriteColors_Blocking((const u16 *)color_p, pixel_count);
}

static void disp_flush_complete(void *user_data)
{
    lv_disp_drv_t *disp_drv = (lv_disp_drv_t *)user_data;

    if (disp_drv != NULL)
    {
        lv_disp_flush_ready(disp_drv);
    }
}

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    lv_coord_t x1 = area->x1;
    lv_coord_t y1 = area->y1;
    lv_coord_t x2 = area->x2;
    lv_coord_t y2 = area->y2;

    if ((x2 < 0) || (y2 < 0) || (x1 > (MY_DISP_HOR_RES - 1)) || (y1 > (MY_DISP_VER_RES - 1)))
    {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    if (x1 < 0)
    {
        x1 = 0;
    }
    if (y1 < 0)
    {
        y1 = 0;
    }
    if (x2 > (MY_DISP_HOR_RES - 1))
    {
        x2 = MY_DISP_HOR_RES - 1;
    }
    if (y2 > (MY_DISP_VER_RES - 1))
    {
        y2 = MY_DISP_VER_RES - 1;
    }

    LCD_Address_Set((u16)x1, (u16)y1, (u16)x2, (u16)y2);

    if ((x1 == area->x1) && (y1 == area->y1) && (x2 == area->x2) && (y2 == area->y2))
    {
        uint32_t pixel_count = (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1);

        if (LCD_WriteColors_DMA((const u16 *)color_p, pixel_count, disp_flush_complete, disp_drv))
        {
            return;
        }

        disp_write_colors(color_p, pixel_count);
    }
    else
    {
        lv_coord_t src_w = area->x2 - area->x1 + 1;
        lv_coord_t y;

        for (y = y1; y <= y2; y++)
        {
            const lv_color_t *row = color_p + (y - area->y1) * src_w + (x1 - area->x1);
            disp_write_colors(row, (uint32_t)(x2 - x1 + 1));
        }
    }

    lv_disp_flush_ready(disp_drv);
}

#else
typedef int keep_pedantic_happy;
#endif
