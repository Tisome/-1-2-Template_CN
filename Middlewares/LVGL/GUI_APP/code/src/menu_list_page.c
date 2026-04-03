#include "menu_list_page.h"

#include <stdio.h>

static void menu_list_page_apply_style(menu_list_page_t *page, uint16_t row_index, bool selected)
{
    lv_color_t bg_color;

    if ((page == NULL) || (row_index >= MENU_VISIBLE_ITEMS))
    {
        return;
    }

    bg_color = selected ? lv_color_hex(MENU_COLOR_PANEL_SEL)
                        : lv_color_hex(MENU_COLOR_PANEL);

    lv_obj_set_style_bg_color(page->item_panels[row_index], bg_color, 0);
    lv_obj_set_style_bg_opa(page->item_panels[row_index], LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(page->item_panels[row_index], selected ? 3 : 0, 0);
    lv_obj_set_style_border_color(page->item_panels[row_index], lv_color_hex(MENU_COLOR_GREEN), 0);
    lv_obj_set_style_text_color(page->item_labels[row_index], lv_color_hex(MENU_COLOR_TEXT), 0);
}

void menu_list_page_create(menu_list_page_t *page, lv_obj_t *parent)
{
    uint16_t row_index;

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

    page->page_label = lv_label_create(page->root);
    lv_obj_set_style_text_font(page->page_label, menu_font_latin_12(), 0);
    lv_obj_set_style_text_color(page->page_label, lv_color_white(), 0);
    lv_obj_align(page->page_label, LV_ALIGN_TOP_RIGHT, -12, 8);

    for (row_index = 0U; row_index < MENU_VISIBLE_ITEMS; row_index++)
    {
        page->item_panels[row_index] = lv_obj_create(page->root);
        lv_obj_remove_style_all(page->item_panels[row_index]);
        lv_obj_set_size(page->item_panels[row_index], 208, 48);
        lv_obj_align(page->item_panels[row_index],
                     LV_ALIGN_TOP_MID,
                     0,
                     (lv_coord_t)(40 + row_index * 60U));
        lv_obj_clear_flag(page->item_panels[row_index], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(page->item_panels[row_index], 14, 0);
        lv_obj_set_style_bg_opa(page->item_panels[row_index], LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(page->item_panels[row_index], lv_color_hex(MENU_COLOR_PANEL), 0);
        lv_obj_set_style_border_width(page->item_panels[row_index], 0, 0);
        lv_obj_set_style_pad_all(page->item_panels[row_index], 0, 0);

        page->item_labels[row_index] = lv_label_create(page->item_panels[row_index]);
        lv_label_set_long_mode(page->item_labels[row_index], LV_LABEL_LONG_DOT);
        lv_obj_set_width(page->item_labels[row_index], 180);
        lv_obj_set_style_text_font(page->item_labels[row_index], menu_font_cn(), 0);
        lv_obj_set_style_text_color(page->item_labels[row_index], lv_color_hex(MENU_COLOR_TEXT), 0);
        lv_obj_set_style_text_align(page->item_labels[row_index], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(page->item_labels[row_index]);
    }
}

void menu_list_page_set_visible(menu_list_page_t *page, bool visible)
{
    if (page == NULL)
    {
        return;
    }

    menu_set_page_hidden(page->root, !visible);
}

void menu_list_page_render(menu_list_page_t *page, const menu_nav_state_t *nav_state)
{
    const menu_page_t *current_page;
    const menu_item_t *items[MENU_VISIBLE_ITEMS] = {0};
    uint16_t total_pages;
    uint16_t current_page_index;
    uint16_t first_index;
    uint16_t visible_count;
    uint16_t row_index;

    if (page == NULL)
    {
        return;
    }

    current_page = menu_nav_get_current_page(nav_state);
    if (current_page == NULL)
    {
        return;
    }

    total_pages = menu_nav_get_total_pages(nav_state);
    current_page_index = menu_nav_get_current_page_index(nav_state);
    if (total_pages == 0U)
    {
        total_pages = 1U;
        current_page_index = 1U;
    }

    (void)snprintf(page->page_text, sizeof(page->page_text), "<%u/%u>", current_page_index, total_pages);
    lv_label_set_text(page->title_label, current_page->title);
    lv_label_set_text(page->page_label, page->page_text);

    visible_count = menu_nav_get_visible_items(nav_state, items, MENU_VISIBLE_ITEMS, &first_index);

    for (row_index = 0U; row_index < MENU_VISIBLE_ITEMS; row_index++)
    {
        if (row_index < visible_count)
        {
            bool selected = (menu_nav_get_selected_index(nav_state) == (uint16_t)(first_index + row_index));

            menu_set_page_hidden(page->item_panels[row_index], false);
            lv_label_set_text(page->item_labels[row_index], items[row_index]->label);
            menu_list_page_apply_style(page, row_index, selected);
        }
        else
        {
            menu_set_page_hidden(page->item_panels[row_index], true);
            lv_label_set_text(page->item_labels[row_index], "");
        }
    }
}
