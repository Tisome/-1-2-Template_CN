#include "lvgl_app_test.h"

#include "bsp_key.h"
#include "menu_demo.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"

#include <stdio.h>

#define LVGL_MENU_VISIBLE_ROWS 4U
#define LVGL_MENU_POLL_PERIOD_MS 30U

static menu_state_t s_menu_state;

static lv_obj_t *s_title_label = NULL;
static lv_obj_t *s_mode_label = NULL;
static lv_obj_t *s_hint_label = NULL;
static lv_obj_t *s_footer_label = NULL;
static lv_obj_t *s_row_panels[LVGL_MENU_VISIBLE_ROWS] = {0};
static lv_obj_t *s_row_labels[LVGL_MENU_VISIBLE_ROWS] = {0};
static char s_row_text[LVGL_MENU_VISIBLE_ROWS][48];
static char s_hint_text[96];

static void lvgl_menu_apply_row_style(uint16_t row_index, bool selected)
{
    lv_color_t bg_color = selected ? lv_color_hex(0x1B998B) : lv_color_hex(0x182028);
    lv_color_t text_color = selected ? lv_color_hex(0x081C15) : lv_color_hex(0xEAF4F4);
    lv_opa_t bg_opa = selected ? LV_OPA_COVER : LV_OPA_30;

    if ((row_index >= LVGL_MENU_VISIBLE_ROWS) || (s_row_panels[row_index] == NULL) || (s_row_labels[row_index] == NULL))
    {
        return;
    }

    lv_obj_set_style_bg_color(s_row_panels[row_index], bg_color, 0);
    lv_obj_set_style_bg_opa(s_row_panels[row_index], bg_opa, 0);
    lv_obj_set_style_border_width(s_row_panels[row_index], selected ? 1U : 0U, 0);
    lv_obj_set_style_border_color(s_row_panels[row_index], lv_color_hex(0x8EF6E4), 0);
    lv_obj_set_style_text_color(s_row_labels[row_index], text_color, 0);
}

static void lvgl_menu_render(void)
{
    const menu_t *current_menu = menu_get_current_menu(&s_menu_state);
    const menu_item_t *selected_item;
    const menu_item_t *visible_items[LVGL_MENU_VISIBLE_ROWS] = {0};
    uint16_t first_index = 0U;
    uint16_t visible_count = 0U;
    uint16_t row_index;
    bool option_menu;

    if (current_menu == NULL)
    {
        return;
    }

    option_menu = menu_current_menu_is_option(&s_menu_state);

    lv_label_set_text(s_title_label, current_menu->title);
    lv_label_set_text(s_mode_label, option_menu ? "Select Value" : "Browse Menu");

    visible_count = menu_get_visible_items(&s_menu_state, visible_items, LVGL_MENU_VISIBLE_ROWS, &first_index);

    for (row_index = 0U; row_index < LVGL_MENU_VISIBLE_ROWS; row_index++)
    {
        if (row_index < visible_count)
        {
            const menu_item_t *item = visible_items[row_index];
            uint16_t absolute_index = (uint16_t)(first_index + row_index);
            bool selected = menu_is_item_selected(&s_menu_state, absolute_index);
            const char *suffix = (item->child != NULL) ? "  >" : "";

            (void)snprintf(s_row_text[row_index],
                           sizeof(s_row_text[row_index]),
                           "%c %s%s",
                           selected ? '>' : ' ',
                           item->label,
                           suffix);

            lv_label_set_text(s_row_labels[row_index], s_row_text[row_index]);
            lv_obj_clear_flag(s_row_panels[row_index], LV_OBJ_FLAG_HIDDEN);
            lvgl_menu_apply_row_style(row_index, selected);
        }
        else
        {
            lv_label_set_text(s_row_labels[row_index], "");
            lv_obj_add_flag(s_row_panels[row_index], LV_OBJ_FLAG_HIDDEN);
        }
    }

    selected_item = menu_get_selected_item(&s_menu_state);
    if ((selected_item != NULL) && (selected_item->hint != NULL))
    {
        (void)snprintf(s_hint_text, sizeof(s_hint_text), "Hint: %s", selected_item->hint);
    }
    else
    {
        (void)snprintf(s_hint_text, sizeof(s_hint_text), "Hint: use the keys to move through the menu");
    }
    lv_label_set_text(s_hint_label, s_hint_text);

    lv_label_set_text(s_footer_label,
                      option_menu ? "KEY1 up   KEY2 down   KEY3 apply   KEY4 back"
                                  : "KEY1 up   KEY2 down   KEY3 enter   KEY4 back");
}

static void lvgl_menu_poll_cb(lv_timer_t *timer)
{
    uint8_t raw_key;
    menu_key_t key;

    LV_UNUSED(timer);

    raw_key = key_scan(0U);
    key = menu_demo_map_key(raw_key);

    if (key == MENU_KEY_NONE)
    {
        return;
    }

    if (menu_process_key(&s_menu_state, key) || menu_is_exit_requested(&s_menu_state))
    {
        if (menu_is_exit_requested(&s_menu_state))
        {
            menu_clear_exit_request(&s_menu_state);
            menu_state_init(&s_menu_state, menu_demo_get_root());
        }

        lvgl_menu_render();
    }
}

static void lvgl_test_create_ui(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *panel;
    lv_obj_t *list_panel;
    uint16_t row_index;

    menu_state_init(&s_menu_state, menu_demo_get_root());

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    panel = lv_obj_create(scr);
    lv_obj_set_size(panel, 220, 220);
    lv_obj_center(panel);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(panel, 16, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x13212B), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x1B998B), 0);
    lv_obj_set_style_pad_all(panel, 12, 0);

    s_title_label = lv_label_create(panel);
    lv_obj_set_style_text_color(s_title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_title_label, &lv_font_montserrat_14, 0);
    lv_obj_align(s_title_label, LV_ALIGN_TOP_LEFT, 0, 0);

    s_mode_label = lv_label_create(panel);
    lv_obj_set_style_text_color(s_mode_label, lv_color_hex(0x8EF6E4), 0);
    lv_obj_set_style_text_font(s_mode_label, &lv_font_montserrat_14, 0);
    lv_obj_align(s_mode_label, LV_ALIGN_TOP_RIGHT, 0, 4);

    list_panel = lv_obj_create(panel);
    lv_obj_set_size(list_panel, 196, 126);
    lv_obj_align(list_panel, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_clear_flag(list_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(list_panel, lv_color_hex(0x0D1820), 0);
    lv_obj_set_style_bg_opa(list_panel, LV_OPA_80, 0);
    lv_obj_set_style_border_width(list_panel, 0, 0);
    lv_obj_set_style_radius(list_panel, 12, 0);
    lv_obj_set_style_pad_all(list_panel, 8, 0);

    for (row_index = 0U; row_index < LVGL_MENU_VISIBLE_ROWS; row_index++)
    {
        s_row_panels[row_index] = lv_obj_create(list_panel);
        lv_obj_set_size(s_row_panels[row_index], 180, 24);
        lv_obj_align(s_row_panels[row_index], LV_ALIGN_TOP_MID, 0, (lv_coord_t)(row_index * 28U));
        lv_obj_clear_flag(s_row_panels[row_index], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(s_row_panels[row_index], 8, 0);
        lv_obj_set_style_pad_left(s_row_panels[row_index], 8, 0);
        lv_obj_set_style_pad_right(s_row_panels[row_index], 8, 0);
        lv_obj_set_style_pad_top(s_row_panels[row_index], 2, 0);
        lv_obj_set_style_pad_bottom(s_row_panels[row_index], 2, 0);

        s_row_labels[row_index] = lv_label_create(s_row_panels[row_index]);
        lv_label_set_long_mode(s_row_labels[row_index], LV_LABEL_LONG_DOT);
        lv_obj_set_width(s_row_labels[row_index], 164);
        lv_obj_set_style_text_font(s_row_labels[row_index], &lv_font_montserrat_14, 0);
        lv_obj_align(s_row_labels[row_index], LV_ALIGN_LEFT_MID, 0, 0);
    }

    s_hint_label = lv_label_create(panel);
    lv_label_set_long_mode(s_hint_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_hint_label, 196);
    lv_obj_set_style_text_color(s_hint_label, lv_color_hex(0xCFE8E8), 0);
    lv_obj_set_style_text_font(s_hint_label, &lv_font_montserrat_14, 0);
    lv_obj_align(s_hint_label, LV_ALIGN_TOP_LEFT, 0, 166);

    s_footer_label = lv_label_create(panel);
    lv_label_set_long_mode(s_footer_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_footer_label, 196);
    lv_obj_set_style_text_color(s_footer_label, lv_color_hex(0x8EF6E4), 0);
    lv_obj_set_style_text_font(s_footer_label, &lv_font_montserrat_14, 0);
    lv_obj_align(s_footer_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lvgl_menu_render();
    lv_timer_create(lvgl_menu_poll_cb, LVGL_MENU_POLL_PERIOD_MS, NULL);
}

void task_lvgl_test(void *p)
{
    (void)p;

    lv_init();
    lv_port_disp_init();
    lvgl_test_create_ui();

    while (1)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}
