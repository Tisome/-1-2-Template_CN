#ifndef MENU_LIST_PAGE_H
#define MENU_LIST_PAGE_H

#include "menu_nav.h"

typedef struct
{
    lv_obj_t *root;
    lv_obj_t *title_label;
    lv_obj_t *page_label;
    lv_obj_t *item_panels[MENU_VISIBLE_ITEMS];
    lv_obj_t *item_labels[MENU_VISIBLE_ITEMS];
    char page_text[16];
} menu_list_page_t;

void menu_list_page_create(menu_list_page_t *page, lv_obj_t *parent);
void menu_list_page_set_visible(menu_list_page_t *page, bool visible);
void menu_list_page_render(menu_list_page_t *page, const menu_nav_state_t *nav_state);

#endif /* MENU_LIST_PAGE_H */
