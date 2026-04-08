/*
 * 设置页渲染文件。
 * 该文件负责把设置会话中的值文本、单位、提示语和高亮位置，
 * 转换成 LVGL 可见控件，属于设置界面的纯显示层。
 */
#include "menu_setting_page.h"

#include "menu_strings.h"
#include <stdio.h>
#include <string.h>

static void menu_setting_page_render_body(menu_setting_page_t *page,
                                          const char *text)
{
    static const lv_coord_t s_body_y[] = {92, 118, 144, 170};
    const char *cursor = text;
    uint16_t line_index;

    if (page == NULL)
    {
        return;
    }

    for (line_index = 0U; line_index < 4U; line_index++)
    {
        page->body_text[line_index][0] = '\0';
        lv_label_set_text_static(page->body_labels[line_index], page->body_text[line_index]);
        menu_set_page_hidden(page->body_labels[line_index], true);
    }

    if (text == NULL)
    {
        return;
    }

    for (line_index = 0U; (line_index < 4U) && (*cursor != '\0'); line_index++)
    {
        size_t copy_len = 0U;

        while ((cursor[copy_len] != '\0') &&
               (cursor[copy_len] != '\n') &&
               (copy_len + 1U < sizeof(page->body_text[line_index])))
        {
            page->body_text[line_index][copy_len] = cursor[copy_len];
            copy_len++;
        }

        page->body_text[line_index][copy_len] = '\0';
        lv_label_set_text_static(page->body_labels[line_index], page->body_text[line_index]);
        lv_obj_align(page->body_labels[line_index], LV_ALIGN_TOP_MID, 0, s_body_y[line_index]);
        menu_set_page_hidden(page->body_labels[line_index], false);

        cursor += copy_len;
        if (*cursor == '\n')
        {
            cursor++;
        }
    }
}

/* 按字符逐位渲染设置值，并高亮当前选中的编辑位。 */
static void menu_setting_page_render_value(menu_setting_page_t *page,
                                           const menu_setting_view_t *view)
{
    uint16_t char_count = 0U;
    uint16_t index;
    lv_coord_t start_x;
    lv_coord_t total_width = 0;

    if ((page == NULL) || (view == NULL))
    {
        if (page != NULL)
        {
            uint16_t index;
            for (index = 0U; index < MENU_SETTING_PAGE_MAX_CHARS; index++)
            {
                page->char_text[index][0] = '\0';
                lv_label_set_text_static(page->char_labels[index], page->char_text[index]);
                menu_set_page_hidden(page->char_boxes[index], true);
            }
        }
        return;
    }

    while ((view->value_text[char_count] != '\0') && (char_count < MENU_SETTING_PAGE_MAX_CHARS))
    {
        total_width += (view->value_text[char_count] == '.') ? 8 : 14;
        char_count++;
    }

    start_x = (lv_coord_t)((190 - total_width) / 2);
    if (start_x < 0)
    {
        start_x = 0;
    }

    for (index = 0U; index < MENU_SETTING_PAGE_MAX_CHARS; index++)
    {
        if (index < char_count)
        {
            lv_coord_t char_width = (view->value_text[index] == '.') ? 8 : 14;
            bool selected = (view->selected_char_index == (int8_t)index);

            page->char_text[index][0] = view->value_text[index];
            page->char_text[index][1] = '\0';

            lv_obj_set_size(page->char_boxes[index], char_width, 24);
            lv_obj_set_pos(page->char_boxes[index], start_x, 3);
            lv_label_set_text_static(page->char_labels[index], page->char_text[index]);
            lv_obj_set_style_bg_opa(page->char_boxes[index], selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_color(page->char_boxes[index], lv_color_white(), 0);
            lv_obj_set_style_text_color(page->char_labels[index],
                                        selected ? lv_color_black() : lv_color_white(),
                                        0);
            menu_set_page_hidden(page->char_boxes[index], false);
            start_x = (lv_coord_t)(start_x + char_width);
        }
        else
        {
            page->char_text[index][0] = '\0';
            lv_label_set_text_static(page->char_labels[index], page->char_text[index]);
            menu_set_page_hidden(page->char_boxes[index], true);
        }
    }
}

/* 创建设置页使用的 LVGL 控件。 */
void menu_setting_page_create(menu_setting_page_t *page, lv_obj_t *parent)
{
    uint16_t index;

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
    lv_label_set_text_static(page->title_label, "");

    page->value_panel = lv_obj_create(page->root);
    lv_obj_remove_style_all(page->value_panel);
    lv_obj_set_size(page->value_panel, 190, 30);
    lv_obj_align(page->value_panel, LV_ALIGN_TOP_MID, -8, 70);
    lv_obj_clear_flag(page->value_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(page->value_panel, 8, 0);
    lv_obj_set_style_bg_color(page->value_panel, lv_color_hex(MENU_COLOR_PANEL), 0);
    lv_obj_set_style_bg_opa(page->value_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(page->value_panel, 0, 0);
    lv_obj_set_style_pad_all(page->value_panel, 0, 0);

    for (index = 0U; index < MENU_SETTING_PAGE_MAX_CHARS; index++)
    {
        page->char_boxes[index] = lv_obj_create(page->value_panel);
        lv_obj_remove_style_all(page->char_boxes[index]);
        lv_obj_clear_flag(page->char_boxes[index], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(page->char_boxes[index], 4, 0);
        lv_obj_set_style_pad_all(page->char_boxes[index], 0, 0);
        lv_obj_set_style_border_width(page->char_boxes[index], 0, 0);
        lv_obj_set_style_bg_opa(page->char_boxes[index], LV_OPA_TRANSP, 0);

        page->char_labels[index] = lv_label_create(page->char_boxes[index]);
        lv_obj_set_style_text_font(page->char_labels[index], menu_font_latin_16(), 0);
        lv_obj_set_style_text_color(page->char_labels[index], lv_color_white(), 0);
        lv_obj_center(page->char_labels[index]);

        page->char_text[index][0] = '\0';
        page->char_text[index][1] = '\0';
        lv_label_set_text_static(page->char_labels[index], page->char_text[index]);
        menu_set_page_hidden(page->char_boxes[index], true);
    }

    page->unit_label = lv_label_create(page->root);
    lv_obj_set_style_text_font(page->unit_label, menu_font_latin_12(), 0);
    lv_obj_set_style_text_color(page->unit_label, lv_color_white(), 0);
    lv_obj_align_to(page->unit_label, page->value_panel, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    lv_label_set_text_static(page->unit_label, "");

    for (index = 0U; index < 4U; index++)
    {
        page->body_labels[index] = lv_label_create(page->root);
        lv_obj_set_width(page->body_labels[index], 210);
        lv_obj_set_style_text_font(page->body_labels[index], menu_font_latin_16(), 0);
        lv_obj_set_style_text_color(page->body_labels[index], lv_color_hex(MENU_COLOR_SUBTEXT), 0);
        lv_obj_set_style_text_align(page->body_labels[index], LV_TEXT_ALIGN_CENTER, 0);
        page->body_text[index][0] = '\0';
        lv_label_set_text_static(page->body_labels[index], page->body_text[index]);
        lv_obj_align(page->body_labels[index], LV_ALIGN_TOP_MID, 0, 92 + (lv_coord_t)(26U * index));
        menu_set_page_hidden(page->body_labels[index], true);
    }

    page->footer_label = lv_label_create(page->root);
    lv_label_set_text_static(page->footer_label, MENU_TXT_BACK);
    lv_obj_set_style_text_font(page->footer_label, menu_font_latin_16(), 0);
    lv_obj_set_style_text_color(page->footer_label, lv_color_white(), 0);
    lv_obj_align(page->footer_label, LV_ALIGN_BOTTOM_MID, 0, -10);
}

/* 显示或隐藏整个设置页。 */
void menu_setting_page_set_visible(menu_setting_page_t *page, bool visible)
{
    if (page == NULL)
    {
        return;
    }

    menu_set_page_hidden(page->root, !visible);
}

/* 按当前设置描述和会话视图刷新页面内容。 */
void menu_setting_page_render(menu_setting_page_t *page,
                              const menu_setting_desc_t *setting,
                              const menu_setting_view_t *view)
{
    if (page == NULL)
    {
        return;
    }

    if (setting == NULL)
    {
        lv_label_set_text_static(page->title_label, "");
        lv_label_set_text_static(page->unit_label, "");
        menu_setting_page_render_body(page, "");
        menu_set_page_hidden(page->value_panel, true);
        menu_set_page_hidden(page->unit_label, true);
        menu_setting_page_render_value(page, NULL);
        return;
    }

    lv_label_set_text_static(page->title_label, setting->title);

    if (view == NULL)
    {
        menu_setting_page_render_body(page, setting->placeholder_text);
        menu_set_page_hidden(page->value_panel, true);
        menu_set_page_hidden(page->unit_label, true);
        menu_setting_page_render_value(page, NULL);
        return;
    }

    if (view->show_value_box)
    {
        menu_setting_page_render_value(page, view);
        lv_label_set_text_static(page->unit_label, view->unit_text);
        menu_set_page_hidden(page->value_panel, false);
        menu_set_page_hidden(page->unit_label, (view->unit_text == NULL) || (view->unit_text[0] == '\0'));
    }
    else
    {
        lv_label_set_text_static(page->unit_label, "");
        menu_setting_page_render_value(page, view);
        menu_set_page_hidden(page->value_panel, true);
        menu_set_page_hidden(page->unit_label, true);
    }

    menu_setting_page_render_body(page, view->detail_text);
}
