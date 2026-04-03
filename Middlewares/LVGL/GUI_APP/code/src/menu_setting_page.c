#include "menu_setting_page.h"

#include "menu_strings.h"

void menu_setting_page_create(menu_setting_page_t *page, lv_obj_t *parent)
{
    if ((page == NULL) || (parent == NULL))
    {
        return;
    }

    page->root = lv_obj_create(parent);
    menu_prepare_page_root(page->root);

    page->title_label = lv_label_create(page->root);
    lv_obj_set_style_text_font(page->title_label, menu_font_cn(), 0);
    lv_obj_set_style_text_color(page->title_label, lv_color_white(), 0);
    lv_obj_align(page->title_label, LV_ALIGN_TOP_LEFT, 14, 8);

    page->body_label = lv_label_create(page->root);
    lv_label_set_long_mode(page->body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(page->body_label, 200);
    lv_obj_set_style_text_font(page->body_label, menu_font_latin_16(), 0);
    lv_obj_set_style_text_color(page->body_label, lv_color_hex(MENU_COLOR_SUBTEXT), 0);
    lv_obj_set_style_text_align(page->body_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(page->body_label, LV_ALIGN_CENTER, 0, -8);

    page->footer_label = lv_label_create(page->root);
    lv_label_set_text(page->footer_label, MENU_TXT_BACK);
    lv_obj_set_style_text_font(page->footer_label, menu_font_latin_16(), 0);
    lv_obj_set_style_text_color(page->footer_label, lv_color_white(), 0);
    lv_obj_align(page->footer_label, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void menu_setting_page_set_visible(menu_setting_page_t *page, bool visible)
{
    if (page == NULL)
    {
        return;
    }

    menu_set_page_hidden(page->root, !visible);
}

void menu_setting_page_render(menu_setting_page_t *page, const menu_setting_desc_t *setting)
{
    if (page == NULL)
    {
        return;
    }

    if (setting == NULL)
    {
        lv_label_set_text(page->title_label, "");
        lv_label_set_text(page->body_label, "");
        return;
    }

    lv_label_set_text(page->title_label, setting->title);
    lv_label_set_text(page->body_label, setting->placeholder_text);
}
