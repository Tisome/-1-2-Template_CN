#ifndef MENU_SETTING_PAGE_H
#define MENU_SETTING_PAGE_H

#include "menu_nav.h"
#include "menu_setting_backend.h"

#define MENU_SETTING_PAGE_MAX_CHARS 12U

typedef struct
{
    lv_obj_t *root;
    lv_obj_t *title_label;
    lv_obj_t *value_panel;
    lv_obj_t *unit_label;
    lv_obj_t *body_label;
    lv_obj_t *footer_label;
    lv_obj_t *char_boxes[MENU_SETTING_PAGE_MAX_CHARS];
    lv_obj_t *char_labels[MENU_SETTING_PAGE_MAX_CHARS];
    char char_text[MENU_SETTING_PAGE_MAX_CHARS][2];
    char body_text[96];
} menu_setting_page_t;

void menu_setting_page_create(menu_setting_page_t *page, lv_obj_t *parent);
void menu_setting_page_set_visible(menu_setting_page_t *page, bool visible);
void menu_setting_page_render(menu_setting_page_t *page,
                              const menu_setting_desc_t *setting,
                              const menu_setting_view_t *view);

#endif /* MENU_SETTING_PAGE_H */
