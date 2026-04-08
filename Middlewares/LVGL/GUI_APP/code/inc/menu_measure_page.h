#ifndef MENU_MEASURE_PAGE_H
#define MENU_MEASURE_PAGE_H

#include "menu_style.h"

typedef struct
{
    lv_obj_t *root;
    lv_obj_t *sq_label;
    lv_obj_t *arc;
    lv_obj_t *value_label;
    lv_obj_t *unit_label;
    lv_obj_t *scale_label;
    lv_obj_t *total_label;
    char sq_text[32];
    char value_text[32];
    char unit_text[16];
    char scale_text[32];
    char total_text[48];
} menu_measure_page_t;

void menu_measure_page_create(menu_measure_page_t *page, lv_obj_t *parent);
void menu_measure_page_set_visible(menu_measure_page_t *page, bool visible);
void menu_measure_page_render(menu_measure_page_t *page,
                              double sq_value,
                              double instant_flow_value,
                              const char *flow_unit_text,
                              double arc_full_scale_value,
                              double total_flow_value,
                              const char *total_unit_text,
                              int16_t arc_value);

#endif /* MENU_MEASURE_PAGE_H */
