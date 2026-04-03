#include "menu_measure_page.h"

#include <stdio.h>

void menu_measure_page_create(menu_measure_page_t *page, lv_obj_t *parent)
{
    lv_obj_t *arc_box;

    if ((page == NULL) || (parent == NULL))
    {
        return;
    }

    page->root = lv_obj_create(parent);
    menu_prepare_page_root(page->root);

    page->sq_label = lv_label_create(page->root);
    lv_obj_set_style_text_font(page->sq_label, menu_font_latin_12(), 0);
    lv_obj_set_style_text_color(page->sq_label, lv_color_white(), 0);
    lv_obj_align(page->sq_label, LV_ALIGN_TOP_LEFT, 10, 6);

    arc_box = lv_obj_create(page->root);
    menu_prepare_page_root(arc_box);
    lv_obj_set_size(arc_box, 190, 190);
    lv_obj_align(arc_box, LV_ALIGN_TOP_MID, 0, 18);

    page->arc = lv_arc_create(arc_box);
    lv_obj_set_size(page->arc, 188, 188);
    lv_obj_center(page->arc);
    lv_arc_set_range(page->arc, 0, MENU_ARC_VALUE_MAX);
    lv_arc_set_bg_angles(page->arc, 130, 50);
    lv_arc_set_value(page->arc, 0);
    lv_obj_clear_flag(page->arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_style(page->arc, NULL, LV_PART_KNOB | LV_STATE_ANY);
    lv_obj_set_style_arc_width(page->arc, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_width(page->arc, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(page->arc, lv_color_hex(MENU_COLOR_GREEN), LV_PART_MAIN);
    lv_obj_set_style_arc_color(page->arc, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(page->arc, true, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(page->arc, true, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(page->arc, LV_OPA_TRANSP, 0);

    page->value_label = lv_label_create(arc_box);
    lv_obj_set_style_text_font(page->value_label, menu_font_latin_28(), 0);
    lv_obj_set_style_text_color(page->value_label, lv_color_white(), 0);
    lv_obj_align(page->value_label, LV_ALIGN_CENTER, 0, -8);

    page->unit_label = lv_label_create(arc_box);
    lv_label_set_text(page->unit_label, "m3/h");
    lv_obj_set_style_text_font(page->unit_label, menu_font_latin_12(), 0);
    lv_obj_set_style_text_color(page->unit_label, lv_color_white(), 0);
    lv_obj_align(page->unit_label, LV_ALIGN_CENTER, 0, 28);

    page->total_label = lv_label_create(page->root);
    lv_obj_set_style_text_font(page->total_label, menu_font_latin_12(), 0);
    lv_obj_set_style_text_color(page->total_label, lv_color_white(), 0);
    lv_obj_align(page->total_label, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void menu_measure_page_set_visible(menu_measure_page_t *page, bool visible)
{
    if (page == NULL)
    {
        return;
    }

    menu_set_page_hidden(page->root, !visible);
}

void menu_measure_page_render(menu_measure_page_t *page,
                              double sq_value,
                              double instant_flow_m3ph,
                              double total_flow_m3,
                              int16_t arc_value)
{
    if (page == NULL)
    {
        return;
    }

    (void)snprintf(page->sq_text, sizeof(page->sq_text), "SQ: %.2f", sq_value);
    (void)snprintf(page->value_text, sizeof(page->value_text), "%06.2f", instant_flow_m3ph);
    (void)snprintf(page->total_text, sizeof(page->total_text), "%.6f  m3", total_flow_m3);

    lv_label_set_text(page->sq_label, page->sq_text);
    lv_label_set_text(page->value_label, page->value_text);
    lv_label_set_text(page->total_label, page->total_text);
    lv_arc_set_value(page->arc, arc_value);
}
