#ifndef MENU_SETTING_PAGE_H
#define MENU_SETTING_PAGE_H

#include "menu_nav.h"

typedef struct
{
    lv_obj_t *root;
    lv_obj_t *title_label;
    lv_obj_t *body_label;
    lv_obj_t *footer_label;
} menu_setting_page_t;

void menu_setting_page_create(menu_setting_page_t *page, lv_obj_t *parent);
void menu_setting_page_set_visible(menu_setting_page_t *page, bool visible);
void menu_setting_page_render(menu_setting_page_t *page, const menu_setting_desc_t *setting);

#endif /* MENU_SETTING_PAGE_H */
