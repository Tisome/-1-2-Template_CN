#ifndef MENU_SETTING_SESSION_H
#define MENU_SETTING_SESSION_H

#include "menu_setting_backend.h"

typedef enum
{
    MENU_SETTING_SESSION_EVENT_NONE = 0,
    MENU_SETTING_SESSION_EVENT_UPDATED,
    MENU_SETTING_SESSION_EVENT_CLOSE,
    MENU_SETTING_SESSION_EVENT_ERROR
} menu_setting_session_event_t;

typedef struct
{
    const menu_setting_desc_t *setting;
    double numeric_value;
    uint32_t option_value;
    bool active;
    bool dirty;
    int8_t selected_digit_index;
    uint8_t digit_positions[16];
    uint8_t digit_count;
    char value_text[32];
    char detail_text[96];
    parameter_apply_status_t last_status;
} menu_setting_session_t;

void menu_setting_session_reset(menu_setting_session_t *session);
bool menu_setting_session_begin(menu_setting_session_t *session, const menu_setting_desc_t *setting);
bool menu_setting_session_is_active(const menu_setting_session_t *session);
bool menu_setting_session_step_up(menu_setting_session_t *session);
bool menu_setting_session_step_down(menu_setting_session_t *session);
menu_setting_session_event_t menu_setting_session_confirm(menu_setting_session_t *session);
bool menu_setting_session_build_view(const menu_setting_session_t *session, menu_setting_view_t *view);

#endif /* MENU_SETTING_SESSION_H */
