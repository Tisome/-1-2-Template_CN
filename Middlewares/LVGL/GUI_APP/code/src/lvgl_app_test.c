#include "lvgl_app_test.h"

#include "bsp_key.h"
#include "data.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"

#include <math.h>
#include <stdio.h>

#define UI_SCREEN_SIZE             240
#define UI_MENU_VISIBLE_ITEMS      3U
#define UI_KEY_POLL_PERIOD_MS      40U
#define UI_NAV_MAX_DEPTH           4U
#define UI_ARC_VALUE_MAX           1000
#define UI_MIN_ARC_FULL_SCALE      0.10

#define UI_COLOR_BLACK             0x000000
#define UI_COLOR_WHITE             0xFFFFFF
#define UI_COLOR_GREEN             0x24FC3D
#define UI_COLOR_MENU_BG           0x11161B
#define UI_COLOR_MENU_PANEL        0x1A2127
#define UI_COLOR_MENU_PANEL_SEL    0x171D22
#define UI_COLOR_MENU_TEXT         0xF3FFFF
#define UI_COLOR_MENU_SUBTEXT      0xB8C8CC

#define TXT_MENU_TITLE             "\xE8\x8F\x9C\xE5\x8D\x95\xE9\xA1\xB9"
#define TXT_MEASURE_DISPLAY        "\xE6\xB5\x8B\xE9\x87\x8F\xE6\x98\xBE\xE7\xA4\xBA"
#define TXT_PIPE_SETTINGS          "\xE7\xAE\xA1\xE9\x81\x93\xE8\xAE\xBE\xE7\xBD\xAE"
#define TXT_OUTPUT_SETTINGS        "\xE8\xBE\x93\xE5\x87\xBA\xE8\xAE\xBE\xE7\xBD\xAE"
#define TXT_ALARM_SETTINGS         "\xE6\x8A\xA5\xE8\xAD\xA6\xE8\xAE\xBE\xE7\xBD\xAE"
#define TXT_SYSTEM_INFO            "\xE7\xB3\xBB\xE7\xBB\x9F\xE4\xBF\xA1\xE6\x81\xAF"
#define TXT_PIPE_OUTER_DIAMETER    "\xE7\xAE\xA1\xE9\x81\x93\xE5\xA4\x96\xE5\xBE\x84"
#define TXT_PIPE_WALL_THICKNESS    "\xE7\xAE\xA1\xE9\x81\x93\xE5\xA3\x81\xE5\x8E\x9A"
#define TXT_PIPE_MATERIAL          "\xE7\xAE\xA1\xE9\x81\x93\xE6\x9D\x90\xE8\xB4\xA8"
#define TXT_OUTPUT_MODE            "\xE8\xBE\x93\xE5\x87\xBA\xE6\xA8\xA1\xE5\xBC\x8F"
#define TXT_FLOW_UNIT              "\xE6\xB5\x81\xE9\x87\x8F\xE5\x8D\x95\xE4\xBD\x8D"
#define TXT_REFRESH_RATE           "\xE5\x88\xB7\xE6\x96\xB0\xE9\xA2\x91\xE7\x8E\x87"
#define TXT_ALARM_LOW              "\xE4\xB8\x8B\xE9\x99\x90\xE6\x8A\xA5\xE8\xAD\xA6"
#define TXT_ALARM_HIGH             "\xE4\xB8\x8A\xE9\x99\x90\xE6\x8A\xA5\xE8\xAD\xA6"
#define TXT_ZERO_LEARN             "\xE9\x9B\xB6\xE7\x82\xB9\xE5\xAD\xA6\xE4\xB9\xA0"
#define TXT_SETTING_PLACEHOLDER    "PLACEHOLDER PAGE\nEDIT IN ANOTHER FILE"
#define TXT_BACK_UP                "K4 BACK"

typedef enum
{
    UI_SCREEN_MEASURE = 0,
    UI_SCREEN_MENU,
    UI_SCREEN_SETTING
} ui_screen_id_t;

typedef enum
{
    UI_KEY_NONE = 0,
    UI_KEY_OK,
    UI_KEY_UP,
    UI_KEY_DOWN,
    UI_KEY_BACK
} ui_key_t;

typedef enum
{
    UI_MENU_ENTRY_MEASURE = 0,
    UI_MENU_ENTRY_SUBMENU,
    UI_MENU_ENTRY_SETTING
} ui_menu_entry_type_t;

typedef struct
{
    const char *title;
    const char *placeholder_text;
} ui_setting_desc_t;

struct ui_menu_page;

typedef struct
{
    const char *label;
    ui_menu_entry_type_t type;
    const struct ui_menu_page *submenu;
    const ui_setting_desc_t *setting;
} ui_menu_item_t;

typedef struct ui_menu_page
{
    const char *title;
    const ui_menu_item_t *items;
    uint16_t item_count;
} ui_menu_page_t;

typedef struct
{
    const ui_menu_page_t *page;
    uint16_t selected_index;
} ui_nav_frame_t;

static const ui_setting_desc_t s_setting_pipe_diameter = {
    TXT_PIPE_OUTER_DIAMETER,
    TXT_SETTING_PLACEHOLDER};

static const ui_setting_desc_t s_setting_pipe_thickness = {
    TXT_PIPE_WALL_THICKNESS,
    TXT_SETTING_PLACEHOLDER};

static const ui_setting_desc_t s_setting_pipe_material = {
    TXT_PIPE_MATERIAL,
    TXT_SETTING_PLACEHOLDER};

static const ui_setting_desc_t s_setting_output_mode = {
    TXT_OUTPUT_MODE,
    TXT_SETTING_PLACEHOLDER};

static const ui_setting_desc_t s_setting_rate_unit = {
    TXT_FLOW_UNIT,
    TXT_SETTING_PLACEHOLDER};

static const ui_setting_desc_t s_setting_refresh_rate = {
    TXT_REFRESH_RATE,
    TXT_SETTING_PLACEHOLDER};

static const ui_setting_desc_t s_setting_alarm_low = {
    TXT_ALARM_LOW,
    TXT_SETTING_PLACEHOLDER};

static const ui_setting_desc_t s_setting_alarm_high = {
    TXT_ALARM_HIGH,
    TXT_SETTING_PLACEHOLDER};

static const ui_setting_desc_t s_setting_zero_learn = {
    TXT_ZERO_LEARN,
    TXT_SETTING_PLACEHOLDER};

static const ui_setting_desc_t s_setting_system_info = {
    TXT_SYSTEM_INFO,
    TXT_SETTING_PLACEHOLDER};

static const ui_menu_page_t s_pipe_menu_page;
static const ui_menu_page_t s_output_menu_page;
static const ui_menu_page_t s_alarm_menu_page;

static const ui_menu_item_t s_pipe_menu_items[] = {
    {TXT_PIPE_OUTER_DIAMETER, UI_MENU_ENTRY_SETTING, NULL, &s_setting_pipe_diameter},
    {TXT_PIPE_WALL_THICKNESS, UI_MENU_ENTRY_SETTING, NULL, &s_setting_pipe_thickness},
    {TXT_PIPE_MATERIAL, UI_MENU_ENTRY_SETTING, NULL, &s_setting_pipe_material},
};

static const ui_menu_item_t s_output_menu_items[] = {
    {TXT_OUTPUT_MODE, UI_MENU_ENTRY_SETTING, NULL, &s_setting_output_mode},
    {TXT_FLOW_UNIT, UI_MENU_ENTRY_SETTING, NULL, &s_setting_rate_unit},
    {TXT_REFRESH_RATE, UI_MENU_ENTRY_SETTING, NULL, &s_setting_refresh_rate},
};

static const ui_menu_item_t s_alarm_menu_items[] = {
    {TXT_ALARM_LOW, UI_MENU_ENTRY_SETTING, NULL, &s_setting_alarm_low},
    {TXT_ALARM_HIGH, UI_MENU_ENTRY_SETTING, NULL, &s_setting_alarm_high},
    {TXT_ZERO_LEARN, UI_MENU_ENTRY_SETTING, NULL, &s_setting_zero_learn},
};

static const ui_menu_page_t s_pipe_menu_page = {
    TXT_PIPE_SETTINGS,
    s_pipe_menu_items,
    (uint16_t)(sizeof(s_pipe_menu_items) / sizeof(s_pipe_menu_items[0]))};

static const ui_menu_page_t s_output_menu_page = {
    TXT_OUTPUT_SETTINGS,
    s_output_menu_items,
    (uint16_t)(sizeof(s_output_menu_items) / sizeof(s_output_menu_items[0]))};

static const ui_menu_page_t s_alarm_menu_page = {
    TXT_ALARM_SETTINGS,
    s_alarm_menu_items,
    (uint16_t)(sizeof(s_alarm_menu_items) / sizeof(s_alarm_menu_items[0]))};

static const ui_menu_item_t s_main_menu_items[] = {
    {TXT_MEASURE_DISPLAY, UI_MENU_ENTRY_MEASURE, NULL, NULL},
    {TXT_PIPE_SETTINGS, UI_MENU_ENTRY_SUBMENU, &s_pipe_menu_page, NULL},
    {TXT_OUTPUT_SETTINGS, UI_MENU_ENTRY_SUBMENU, &s_output_menu_page, NULL},
    {TXT_ALARM_SETTINGS, UI_MENU_ENTRY_SUBMENU, &s_alarm_menu_page, NULL},
    {TXT_SYSTEM_INFO, UI_MENU_ENTRY_SETTING, NULL, &s_setting_system_info},
};

static const ui_menu_page_t s_main_menu_page = {
    TXT_MENU_TITLE,
    s_main_menu_items,
    (uint16_t)(sizeof(s_main_menu_items) / sizeof(s_main_menu_items[0]))};

static ui_screen_id_t s_current_screen = UI_SCREEN_MEASURE;
static ui_nav_frame_t s_nav_stack[UI_NAV_MAX_DEPTH];
static uint16_t s_nav_depth = 0U;
static const ui_setting_desc_t *s_active_setting = NULL;

static lv_obj_t *s_measure_page = NULL;
static lv_obj_t *s_measure_sq_label = NULL;
static lv_obj_t *s_measure_arc = NULL;
static lv_obj_t *s_measure_value_label = NULL;
static lv_obj_t *s_measure_unit_label = NULL;
static lv_obj_t *s_measure_total_label = NULL;

static lv_obj_t *s_menu_page = NULL;
static lv_obj_t *s_menu_title_label = NULL;
static lv_obj_t *s_menu_page_label = NULL;
static lv_obj_t *s_menu_item_panels[UI_MENU_VISIBLE_ITEMS] = {0};
static lv_obj_t *s_menu_item_labels[UI_MENU_VISIBLE_ITEMS] = {0};

static lv_obj_t *s_setting_page = NULL;
static lv_obj_t *s_setting_title_label = NULL;
static lv_obj_t *s_setting_body_label = NULL;
static lv_obj_t *s_setting_footer_label = NULL;

static char s_measure_sq_text[32];
static char s_measure_value_text[32];
static char s_measure_total_text[48];
static char s_menu_page_text[16];

static const lv_font_t *ui_get_cn_font(void)
{
    return &my_chinese_font_size_14_caiti;
}

static const lv_font_t *ui_get_latin_font_12(void)
{
    return &my_num_abc_font_size_12_regular;
}

static const lv_font_t *ui_get_latin_font_16(void)
{
    return &my_num_abc_font_size_16_regular;
}

static const lv_font_t *ui_get_latin_font_28(void)
{
    return &my_num_font_size_28_regular;
}

static ui_key_t ui_map_key(uint8_t raw_key)
{
    switch (raw_key)
    {
    case KEY1_PRESS:
    case KEY1_LONG_PRESS:
        return UI_KEY_OK;

    case KEY2_PRESS:
    case KEY2_LONG_PRESS:
        return UI_KEY_UP;

    case KEY3_PRESS:
    case KEY3_LONG_PRESS:
        return UI_KEY_DOWN;

    case KEY4_PRESS:
    case KEY4_LONG_PRESS:
        return UI_KEY_BACK;

    default:
        return UI_KEY_NONE;
    }
}

static double ui_rate_to_m3ps(double rate_value, RateUnitType unit)
{
    switch (unit)
    {
    case RATE_UNIT_M3_P_H:
        return rate_value / 3600.0;

    case RATE_UNIT_M3_P_MIN:
        return rate_value / 60.0;

    case RATE_UNIT_L_P_H:
        return rate_value / (1000.0 * 3600.0);

    case RATE_UNIT_L_P_MIN:
        return rate_value / (1000.0 * 60.0);

    case RATE_UNIT_L_P_S:
        return rate_value / 1000.0;

    case RATE_UNIT_M3_P_S:
    default:
        return rate_value;
    }
}

static double ui_get_instant_flow_m3ph(void)
{
    double flow_m3ps = ui_rate_to_m3ps(g_algo_out.flow_rate_instant, g_algo_out.flow_rate_unit);

    return convert_rate_from_m3ps(flow_m3ps, RATE_UNIT_M3_P_H);
}

static double ui_get_arc_full_scale_m3ph(void)
{
    double full_scale_m3ps = ui_rate_to_m3ps(g_parameters.alarm_upper_rate_range, g_parameters.rate_unit_type);
    double full_scale_m3ph = fabs(convert_rate_from_m3ps(full_scale_m3ps, RATE_UNIT_M3_P_H));

    if (full_scale_m3ph < UI_MIN_ARC_FULL_SCALE)
    {
        full_scale_m3ph = 1.0;
    }

    return full_scale_m3ph;
}

static uint16_t ui_get_menu_page_count(uint16_t item_count)
{
    return (uint16_t)((item_count + UI_MENU_VISIBLE_ITEMS - 1U) / UI_MENU_VISIBLE_ITEMS);
}

static ui_nav_frame_t *ui_get_current_frame(void)
{
    if (s_nav_depth == 0U)
    {
        return NULL;
    }

    return &s_nav_stack[s_nav_depth - 1U];
}

static const ui_nav_frame_t *ui_get_current_frame_const(void)
{
    if (s_nav_depth == 0U)
    {
        return NULL;
    }

    return &s_nav_stack[s_nav_depth - 1U];
}

static void ui_nav_reset_root(void)
{
    s_nav_depth = 1U;
    s_nav_stack[0].page = &s_main_menu_page;
    s_nav_stack[0].selected_index = 0U;
    s_active_setting = NULL;
}

static void ui_hide_all_pages(void)
{
    if (s_measure_page != NULL)
    {
        lv_obj_add_flag(s_measure_page, LV_OBJ_FLAG_HIDDEN);
    }

    if (s_menu_page != NULL)
    {
        lv_obj_add_flag(s_menu_page, LV_OBJ_FLAG_HIDDEN);
    }

    if (s_setting_page != NULL)
    {
        lv_obj_add_flag(s_setting_page, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ui_show_measure_screen(void)
{
    s_current_screen = UI_SCREEN_MEASURE;
    ui_hide_all_pages();
    lv_obj_clear_flag(s_measure_page, LV_OBJ_FLAG_HIDDEN);
}

static void ui_show_menu_screen(void)
{
    s_current_screen = UI_SCREEN_MENU;
    ui_hide_all_pages();
    lv_obj_clear_flag(s_menu_page, LV_OBJ_FLAG_HIDDEN);
}

static void ui_show_setting_screen(const ui_setting_desc_t *setting)
{
    if (setting == NULL)
    {
        return;
    }

    s_active_setting = setting;
    s_current_screen = UI_SCREEN_SETTING;
    ui_hide_all_pages();
    lv_obj_clear_flag(s_setting_page, LV_OBJ_FLAG_HIDDEN);
}

static void ui_apply_menu_item_style(uint16_t row_index, bool selected)
{
    lv_color_t border_color = lv_color_hex(UI_COLOR_GREEN);
    lv_color_t text_color = lv_color_hex(UI_COLOR_MENU_TEXT);
    lv_color_t bg_color = selected ? lv_color_hex(UI_COLOR_MENU_PANEL_SEL) : lv_color_hex(UI_COLOR_MENU_PANEL);

    if ((row_index >= UI_MENU_VISIBLE_ITEMS) || (s_menu_item_panels[row_index] == NULL) || (s_menu_item_labels[row_index] == NULL))
    {
        return;
    }

    lv_obj_set_style_bg_color(s_menu_item_panels[row_index], bg_color, 0);
    lv_obj_set_style_bg_opa(s_menu_item_panels[row_index], LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_menu_item_panels[row_index], selected ? 3 : 0, 0);
    lv_obj_set_style_border_color(s_menu_item_panels[row_index], border_color, 0);
    lv_obj_set_style_text_color(s_menu_item_labels[row_index], text_color, 0);
}

static void ui_render_measure_page(void)
{
    double sq_value = g_algo_out.sq_value;
    double instant_flow_m3ph = ui_get_instant_flow_m3ph();
    double total_flow_m3 = g_algo_state.q_total_m3;
    double full_scale_m3ph = ui_get_arc_full_scale_m3ph();
    double ratio;
    int16_t arc_value;

    if (sq_value < 0.0)
    {
        sq_value = 0.0;
    }
    else if (sq_value > 100.0)
    {
        sq_value = 100.0;
    }

    ratio = fabs(instant_flow_m3ph) / full_scale_m3ph;
    if (ratio > 1.0)
    {
        ratio = 1.0;
    }

    arc_value = (int16_t)(ratio * (double)UI_ARC_VALUE_MAX + 0.5);

    (void)snprintf(s_measure_sq_text, sizeof(s_measure_sq_text), "SQ: %.2f", sq_value);
    (void)snprintf(s_measure_value_text, sizeof(s_measure_value_text), "%06.2f", instant_flow_m3ph);
    (void)snprintf(s_measure_total_text, sizeof(s_measure_total_text), "%.6f  m3", total_flow_m3);

    lv_label_set_text(s_measure_sq_label, s_measure_sq_text);
    lv_label_set_text(s_measure_value_label, s_measure_value_text);
    lv_label_set_text(s_measure_total_label, s_measure_total_text);
    lv_arc_set_value(s_measure_arc, arc_value);
}

static void ui_render_menu_page(void)
{
    const ui_nav_frame_t *frame = ui_get_current_frame_const();
    uint16_t page_start;
    uint16_t total_pages;
    uint16_t current_page;
    uint16_t row_index;

    if ((frame == NULL) || (frame->page == NULL))
    {
        return;
    }

    page_start = (uint16_t)((frame->selected_index / UI_MENU_VISIBLE_ITEMS) * UI_MENU_VISIBLE_ITEMS);
    total_pages = ui_get_menu_page_count(frame->page->item_count);
    current_page = (uint16_t)(page_start / UI_MENU_VISIBLE_ITEMS + 1U);

    (void)snprintf(s_menu_page_text, sizeof(s_menu_page_text), "<%u/%u>", current_page, total_pages);

    lv_label_set_text(s_menu_title_label, frame->page->title);
    lv_label_set_text(s_menu_page_label, s_menu_page_text);

    for (row_index = 0U; row_index < UI_MENU_VISIBLE_ITEMS; row_index++)
    {
        uint16_t absolute_index = (uint16_t)(page_start + row_index);

        if (absolute_index < frame->page->item_count)
        {
            bool selected = (absolute_index == frame->selected_index);

            lv_obj_clear_flag(s_menu_item_panels[row_index], LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(s_menu_item_labels[row_index], frame->page->items[absolute_index].label);
            ui_apply_menu_item_style(row_index, selected);
        }
        else
        {
            lv_obj_add_flag(s_menu_item_panels[row_index], LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(s_menu_item_labels[row_index], "");
        }
    }
}

static void ui_render_setting_page(void)
{
    if (s_active_setting == NULL)
    {
        return;
    }

    lv_label_set_text(s_setting_title_label, s_active_setting->title);
    lv_label_set_text(s_setting_body_label, s_active_setting->placeholder_text);
}

static void ui_enter_submenu(const ui_menu_page_t *submenu)
{
    if ((submenu == NULL) || (s_nav_depth >= UI_NAV_MAX_DEPTH))
    {
        return;
    }

    s_nav_stack[s_nav_depth].page = submenu;
    s_nav_stack[s_nav_depth].selected_index = 0U;
    s_nav_depth++;

    ui_show_menu_screen();
    ui_render_menu_page();
}

static void ui_open_setting(const ui_setting_desc_t *setting)
{
    ui_show_setting_screen(setting);
    ui_render_setting_page();
}

static void ui_back_to_measure(void)
{
    ui_nav_reset_root();
    ui_show_measure_screen();
    ui_render_measure_page();
}

static void ui_back_to_parent_menu(void)
{
    if (s_nav_depth > 1U)
    {
        s_nav_depth--;
        s_nav_stack[s_nav_depth - 1U].selected_index = 0U;
    }

    ui_show_menu_screen();
    ui_render_menu_page();
}

static void ui_back_from_setting(void)
{
    ui_nav_frame_t *frame = ui_get_current_frame();

    if (frame != NULL)
    {
        frame->selected_index = 0U;
    }

    s_active_setting = NULL;
    ui_show_menu_screen();
    ui_render_menu_page();
}

static void ui_handle_menu_ok(void)
{
    const ui_nav_frame_t *frame = ui_get_current_frame_const();
    const ui_menu_item_t *item;

    if ((frame == NULL) || (frame->page == NULL) || (frame->selected_index >= frame->page->item_count))
    {
        return;
    }

    item = &frame->page->items[frame->selected_index];

    switch (item->type)
    {
    case UI_MENU_ENTRY_MEASURE:
        ui_back_to_measure();
        break;

    case UI_MENU_ENTRY_SUBMENU:
        ui_enter_submenu(item->submenu);
        break;

    case UI_MENU_ENTRY_SETTING:
        ui_open_setting(item->setting);
        break;

    default:
        break;
    }
}

static void ui_handle_key(ui_key_t key)
{
    ui_nav_frame_t *frame = ui_get_current_frame();

    switch (s_current_screen)
    {
    case UI_SCREEN_MEASURE:
        if (key == UI_KEY_BACK)
        {
            ui_nav_reset_root();
            ui_show_menu_screen();
            ui_render_menu_page();
        }
        break;

    case UI_SCREEN_MENU:
        if ((frame == NULL) || (frame->page == NULL))
        {
            return;
        }

        switch (key)
        {
        case UI_KEY_UP:
            if (frame->selected_index > 0U)
            {
                frame->selected_index--;
                ui_render_menu_page();
            }
            break;

        case UI_KEY_DOWN:
            if ((frame->selected_index + 1U) < frame->page->item_count)
            {
                frame->selected_index++;
                ui_render_menu_page();
            }
            break;

        case UI_KEY_OK:
            ui_handle_menu_ok();
            break;

        case UI_KEY_BACK:
            if (s_nav_depth > 1U)
            {
                ui_back_to_parent_menu();
            }
            else
            {
                ui_back_to_measure();
            }
            break;

        case UI_KEY_NONE:
        default:
            break;
        }
        break;

    case UI_SCREEN_SETTING:
        if (key == UI_KEY_BACK)
        {
            ui_back_from_setting();
        }
        break;

    default:
        break;
    }
}

static void ui_init_page_root(lv_obj_t *obj)
{
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, UI_SCREEN_SIZE, UI_SCREEN_SIZE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

static void ui_create_measure_page(lv_obj_t *parent)
{
    lv_obj_t *arc_box;

    s_measure_page = lv_obj_create(parent);
    ui_init_page_root(s_measure_page);

    s_measure_sq_label = lv_label_create(s_measure_page);
    lv_obj_set_style_text_font(s_measure_sq_label, ui_get_latin_font_12(), 0);
    lv_obj_set_style_text_color(s_measure_sq_label, lv_color_white(), 0);
    lv_obj_align(s_measure_sq_label, LV_ALIGN_TOP_LEFT, 10, 6);

    arc_box = lv_obj_create(s_measure_page);
    ui_init_page_root(arc_box);
    lv_obj_set_size(arc_box, 190, 190);
    lv_obj_align(arc_box, LV_ALIGN_TOP_MID, 0, 18);

    s_measure_arc = lv_arc_create(arc_box);
    lv_obj_set_size(s_measure_arc, 188, 188);
    lv_obj_center(s_measure_arc);
    lv_arc_set_range(s_measure_arc, 0, UI_ARC_VALUE_MAX);
    lv_arc_set_bg_angles(s_measure_arc, 130, 50);
    lv_arc_set_value(s_measure_arc, 0);
    lv_obj_clear_flag(s_measure_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_style(s_measure_arc, NULL, LV_PART_KNOB | LV_STATE_ANY);
    lv_obj_set_style_arc_width(s_measure_arc, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_measure_arc, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_measure_arc, lv_color_hex(UI_COLOR_GREEN), LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_measure_arc, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(s_measure_arc, true, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(s_measure_arc, true, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_measure_arc, LV_OPA_TRANSP, 0);

    s_measure_value_label = lv_label_create(arc_box);
    lv_obj_set_style_text_font(s_measure_value_label, ui_get_latin_font_28(), 0);
    lv_obj_set_style_text_color(s_measure_value_label, lv_color_white(), 0);
    lv_obj_align(s_measure_value_label, LV_ALIGN_CENTER, 0, -8);

    s_measure_unit_label = lv_label_create(arc_box);
    lv_label_set_text(s_measure_unit_label, "m3/h");
    lv_obj_set_style_text_font(s_measure_unit_label, ui_get_latin_font_12(), 0);
    lv_obj_set_style_text_color(s_measure_unit_label, lv_color_white(), 0);
    lv_obj_align(s_measure_unit_label, LV_ALIGN_CENTER, 0, 28);

    s_measure_total_label = lv_label_create(s_measure_page);
    lv_obj_set_style_text_font(s_measure_total_label, ui_get_latin_font_12(), 0);
    lv_obj_set_style_text_color(s_measure_total_label, lv_color_white(), 0);
    lv_obj_align(s_measure_total_label, LV_ALIGN_BOTTOM_MID, 0, -10);
}

static void ui_create_menu_page(lv_obj_t *parent)
{
    uint16_t row_index;

    s_menu_page = lv_obj_create(parent);
    ui_init_page_root(s_menu_page);

    s_menu_title_label = lv_label_create(s_menu_page);
    lv_obj_set_style_text_font(s_menu_title_label, ui_get_cn_font(), 0);
    lv_obj_set_style_text_color(s_menu_title_label, lv_color_white(), 0);
    lv_obj_align(s_menu_title_label, LV_ALIGN_TOP_LEFT, 14, 8);

    s_menu_page_label = lv_label_create(s_menu_page);
    lv_obj_set_style_text_font(s_menu_page_label, ui_get_latin_font_12(), 0);
    lv_obj_set_style_text_color(s_menu_page_label, lv_color_white(), 0);
    lv_obj_align(s_menu_page_label, LV_ALIGN_TOP_RIGHT, -12, 8);

    for (row_index = 0U; row_index < UI_MENU_VISIBLE_ITEMS; row_index++)
    {
        s_menu_item_panels[row_index] = lv_obj_create(s_menu_page);
        lv_obj_remove_style_all(s_menu_item_panels[row_index]);
        lv_obj_set_size(s_menu_item_panels[row_index], 208, 48);
        lv_obj_align(s_menu_item_panels[row_index],
                     LV_ALIGN_TOP_MID,
                     0,
                     (lv_coord_t)(40 + row_index * 60U));
        lv_obj_clear_flag(s_menu_item_panels[row_index], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(s_menu_item_panels[row_index], 14, 0);
        lv_obj_set_style_bg_opa(s_menu_item_panels[row_index], LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(s_menu_item_panels[row_index], lv_color_hex(UI_COLOR_MENU_PANEL), 0);
        lv_obj_set_style_border_width(s_menu_item_panels[row_index], 0, 0);
        lv_obj_set_style_pad_all(s_menu_item_panels[row_index], 0, 0);

        s_menu_item_labels[row_index] = lv_label_create(s_menu_item_panels[row_index]);
        lv_label_set_long_mode(s_menu_item_labels[row_index], LV_LABEL_LONG_DOT);
        lv_obj_set_width(s_menu_item_labels[row_index], 180);
        lv_obj_set_style_text_font(s_menu_item_labels[row_index], ui_get_cn_font(), 0);
        lv_obj_set_style_text_color(s_menu_item_labels[row_index], lv_color_hex(UI_COLOR_MENU_TEXT), 0);
        lv_obj_set_style_text_align(s_menu_item_labels[row_index], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(s_menu_item_labels[row_index]);
    }
}

static void ui_create_setting_page(lv_obj_t *parent)
{
    s_setting_page = lv_obj_create(parent);
    ui_init_page_root(s_setting_page);

    s_setting_title_label = lv_label_create(s_setting_page);
    lv_obj_set_style_text_font(s_setting_title_label, ui_get_cn_font(), 0);
    lv_obj_set_style_text_color(s_setting_title_label, lv_color_white(), 0);
    lv_obj_align(s_setting_title_label, LV_ALIGN_TOP_LEFT, 14, 8);

    s_setting_body_label = lv_label_create(s_setting_page);
    lv_label_set_long_mode(s_setting_body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_setting_body_label, 200);
    lv_obj_set_style_text_font(s_setting_body_label, ui_get_latin_font_16(), 0);
    lv_obj_set_style_text_color(s_setting_body_label, lv_color_hex(UI_COLOR_MENU_SUBTEXT), 0);
    lv_obj_set_style_text_align(s_setting_body_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_setting_body_label, LV_ALIGN_CENTER, 0, -8);

    s_setting_footer_label = lv_label_create(s_setting_page);
    lv_label_set_text(s_setting_footer_label, TXT_BACK_UP);
    lv_obj_set_style_text_font(s_setting_footer_label, ui_get_latin_font_16(), 0);
    lv_obj_set_style_text_color(s_setting_footer_label, lv_color_white(), 0);
    lv_obj_align(s_setting_footer_label, LV_ALIGN_BOTTOM_MID, 0, -10);
}

static void ui_create(void)
{
    lv_obj_t *scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, lv_color_hex(UI_COLOR_BLACK), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    ui_create_measure_page(scr);
    ui_create_menu_page(scr);
    ui_create_setting_page(scr);

    ui_hide_all_pages();
    ui_nav_reset_root();
    ui_show_measure_screen();
    ui_render_measure_page();
}

static void ui_tick_cb(lv_timer_t *timer)
{
    ui_key_t key;

    LV_UNUSED(timer);

    key = ui_map_key(key_scan(0U));

    if (key != UI_KEY_NONE)
    {
        ui_handle_key(key);
    }

    if (s_current_screen == UI_SCREEN_MEASURE)
    {
        ui_render_measure_page();
    }
}

void task_lvgl_test(void *p)
{
    (void)p;

    lv_init();
    lv_port_disp_init();
    ui_create();
    lv_timer_create(ui_tick_cb, UI_KEY_POLL_PERIOD_MS, NULL);

    while (1)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}
