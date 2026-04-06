/**
 * @file lv_port_disp_templ.c
 */

#if 1

#include "lv_port_disp_template.h"
#include "../../lvgl.h"
#include "lcd.h"
#include "lcd_init.h"

#include <stdint.h>

#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 240
#define DISP_BUF_LINES 3

typedef struct
{
    uint32_t flush_enter_count;
    uint32_t flush_exit_count;
    uint32_t active_state;
    uint32_t last_start_tick;
    uint32_t last_end_tick;
    uint32_t current_wait_loops;
    uint32_t last_wait_loops;
    uint32_t max_wait_loops;
    uint32_t last_pixel_count;
    uint32_t dma_start_count;
    uint32_t dma_fallback_count;
    uint32_t clipped_flush_count;
    int32_t last_x1;
    int32_t last_y1;
    int32_t last_x2;
    int32_t last_y2;
} lvgl_display_diag_t;

/* Visible in Ozone to determine whether the GUI is stuck in flush/DMA wait. */
volatile lvgl_display_diag_t g_lvgl_display_diag;

static void disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
static void disp_write_colors(const lv_color_t *color_p, uint32_t pixel_count);

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

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    lv_coord_t x1 = area->x1;
    lv_coord_t y1 = area->y1;
    lv_coord_t x2 = area->x2;
    lv_coord_t y2 = area->y2;

    g_lvgl_display_diag.flush_enter_count++;
    g_lvgl_display_diag.active_state = 1U;
    g_lvgl_display_diag.last_start_tick = lv_tick_get();
    g_lvgl_display_diag.current_wait_loops = 0U;
    g_lvgl_display_diag.last_wait_loops = 0U;
    g_lvgl_display_diag.last_pixel_count = 0U;

    if ((x2 < 0) || (y2 < 0) || (x1 > (MY_DISP_HOR_RES - 1)) || (y1 > (MY_DISP_VER_RES - 1)))
    {
        g_lvgl_display_diag.last_x1 = (int32_t)x1;
        g_lvgl_display_diag.last_y1 = (int32_t)y1;
        g_lvgl_display_diag.last_x2 = (int32_t)x2;
        g_lvgl_display_diag.last_y2 = (int32_t)y2;
        g_lvgl_display_diag.last_end_tick = lv_tick_get();
        g_lvgl_display_diag.active_state = 0U;
        g_lvgl_display_diag.flush_exit_count++;
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

    g_lvgl_display_diag.last_x1 = (int32_t)x1;
    g_lvgl_display_diag.last_y1 = (int32_t)y1;
    g_lvgl_display_diag.last_x2 = (int32_t)x2;
    g_lvgl_display_diag.last_y2 = (int32_t)y2;

    LCD_Address_Set((u16)x1, (u16)y1, (u16)x2, (u16)y2);

    if ((x1 == area->x1) && (y1 == area->y1) && (x2 == area->x2) && (y2 == area->y2))
    {
        uint32_t pixel_count = (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1);

        g_lvgl_display_diag.last_pixel_count = pixel_count;

        /* Finish the transfer in task context so LVGL isn't touched from DMA ISR. */
        if (LCD_WriteColors_DMA((const u16 *)color_p, pixel_count, NULL, NULL))
        {
            uint32_t wait_loops = 0U;

            g_lvgl_display_diag.active_state = 2U;
            g_lvgl_display_diag.dma_start_count++;

            while (LCD_DMA_IsBusy())
            {
                wait_loops++;

                if ((wait_loops & 0xFFU) == 0U)
                {
                    g_lvgl_display_diag.current_wait_loops = wait_loops;
                }
            }

            g_lvgl_display_diag.current_wait_loops = wait_loops;
            g_lvgl_display_diag.last_wait_loops = wait_loops;
            if (wait_loops > g_lvgl_display_diag.max_wait_loops)
            {
                g_lvgl_display_diag.max_wait_loops = wait_loops;
            }
        }
        else
        {
            g_lvgl_display_diag.active_state = 3U;
            g_lvgl_display_diag.dma_fallback_count++;
            disp_write_colors(color_p, pixel_count);
        }
    }
    else
    {
        lv_coord_t src_w = area->x2 - area->x1 + 1;
        lv_coord_t y;

        g_lvgl_display_diag.active_state = 4U;
        g_lvgl_display_diag.clipped_flush_count++;
        g_lvgl_display_diag.last_pixel_count = (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1);

        for (y = y1; y <= y2; y++)
        {
            const lv_color_t *row = color_p + (y - area->y1) * src_w + (x1 - area->x1);
            disp_write_colors(row, (uint32_t)(x2 - x1 + 1));
        }
    }

    g_lvgl_display_diag.last_end_tick = lv_tick_get();
    g_lvgl_display_diag.active_state = 0U;
    g_lvgl_display_diag.flush_exit_count++;
    lv_disp_flush_ready(disp_drv);
}

#else
typedef int keep_pedantic_happy;
#endif
