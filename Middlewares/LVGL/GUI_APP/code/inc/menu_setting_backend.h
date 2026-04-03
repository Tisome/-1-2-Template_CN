#ifndef MENU_SETTING_BACKEND_H
#define MENU_SETTING_BACKEND_H

#include "data.h"
#include "menu_nav.h"

typedef struct
{
    bool show_value_box;
    char value_text[32];
    const char *unit_text;
    char detail_text[96];
    int8_t selected_char_index;
} menu_setting_view_t;

bool menu_setting_backend_build_view(const menu_setting_desc_t *setting,
                                     menu_setting_view_t *view);
bool menu_setting_backend_get_numeric_value(menu_setting_id_t id, double *value);
bool menu_setting_backend_get_option_value(menu_setting_id_t id, uint32_t *value);
parameter_apply_status_t menu_setting_backend_commit_numeric(menu_setting_id_t id, double value);
parameter_apply_status_t menu_setting_backend_commit_option(menu_setting_id_t id, uint32_t value);
parameter_apply_status_t menu_setting_backend_step_option(const menu_setting_desc_t *setting, int8_t step);
parameter_apply_status_t menu_setting_backend_execute_action(menu_setting_id_t id);

#endif /* MENU_SETTING_BACKEND_H */
