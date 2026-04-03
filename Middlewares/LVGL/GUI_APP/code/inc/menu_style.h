#ifndef MENU_STYLE_H
#define MENU_STYLE_H

#include <stdbool.h>

#include "lvgl.h"

#define MENU_SCREEN_SIZE        240
#define MENU_VISIBLE_ITEMS      3U
#define MENU_KEY_POLL_PERIOD_MS 40U
#define MENU_NAV_MAX_DEPTH      4U
#define MENU_ARC_VALUE_MAX      1000
#define MENU_MIN_ARC_FULL_SCALE 0.10

#define MENU_COLOR_BLACK          0x000000
#define MENU_COLOR_WHITE          0xFFFFFF
#define MENU_COLOR_GREEN          0x24FC3D
#define MENU_COLOR_PANEL          0x1A2127
#define MENU_COLOR_PANEL_SEL      0x171D22
#define MENU_COLOR_TEXT           0xF3FFFF
#define MENU_COLOR_SUBTEXT        0xB8C8CC

static inline const lv_font_t *menu_font_cn(void)
{
    return &my_chinese_font_size_14_caiti;
}

static inline const lv_font_t *menu_font_latin_12(void)
{
    return &my_num_abc_font_size_12_regular;
}

static inline const lv_font_t *menu_font_latin_16(void)
{
    return &my_num_abc_font_size_16_regular;
}

static inline const lv_font_t *menu_font_latin_28(void)
{
    return &my_num_font_size_28_regular;
}

static inline void menu_prepare_page_root(lv_obj_t *obj)
{
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, MENU_SCREEN_SIZE, MENU_SCREEN_SIZE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

static inline void menu_set_page_hidden(lv_obj_t *obj, bool hidden)
{
    if (obj == NULL)
    {
        return;
    }

    if (hidden)
    {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

#endif /* MENU_STYLE_H */
