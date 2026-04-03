#ifndef MENU_NAV_H
#define MENU_NAV_H

#include <stdbool.h>
#include <stdint.h>

#include "menu_style.h"

typedef enum
{
    MENU_ENTRY_MEASURE = 0,
    MENU_ENTRY_SUBMENU,
    MENU_ENTRY_SETTING
} menu_entry_type_t;

typedef enum
{
    MENU_SETTING_ID_NONE = 0,
    MENU_SETTING_ID_PIPE_OD,
    MENU_SETTING_ID_PIPE_WALL,
    MENU_SETTING_ID_PIPE_MATERIAL,
    MENU_SETTING_ID_OUTPUT_MODE,
    MENU_SETTING_ID_FLOW_UNIT,
    MENU_SETTING_ID_REFRESH_RATE,
    MENU_SETTING_ID_ALARM_LOW,
    MENU_SETTING_ID_ALARM_HIGH,
    MENU_SETTING_ID_ZERO_LEARN,
    MENU_SETTING_ID_SYSTEM
} menu_setting_id_t;

typedef enum
{
    MENU_SETTING_KIND_INFO = 0,
    MENU_SETTING_KIND_NUMERIC,
    MENU_SETTING_KIND_OPTION,
    MENU_SETTING_KIND_ACTION
} menu_setting_kind_t;

typedef struct
{
    const char *label;
    uint32_t value;
} menu_setting_option_t;

typedef struct
{
    uint8_t integer_digits;
    uint8_t fractional_digits;
    const char *unit_text;
    double min_value;
    double max_value;
} menu_setting_numeric_spec_t;

typedef struct
{
    const menu_setting_option_t *options;
    uint16_t option_count;
} menu_setting_option_spec_t;

typedef struct
{
    menu_setting_id_t id;
    const char *title;
    menu_setting_kind_t kind;
    const char *placeholder_text;
    menu_setting_numeric_spec_t numeric;
    menu_setting_option_spec_t option;
} menu_setting_desc_t;

struct menu_page;

typedef struct
{
    const char *label;
    menu_entry_type_t type;
    const struct menu_page *submenu;
    const menu_setting_desc_t *setting;
} menu_item_t;

typedef struct menu_page
{
    const char *title;
    const menu_item_t *items;
    uint16_t item_count;
    uint16_t items_per_page;
    uint16_t default_index;
} menu_page_t;

typedef struct
{
    const menu_page_t *page;
    uint16_t selected_index;
} menu_nav_frame_t;

typedef struct
{
    menu_nav_frame_t frames[MENU_NAV_MAX_DEPTH];
    uint16_t depth;
} menu_nav_state_t;

void menu_nav_init(menu_nav_state_t *state, const menu_page_t *root_page);
void menu_nav_reset_root(menu_nav_state_t *state, const menu_page_t *root_page);
void menu_nav_reset_current_selection(menu_nav_state_t *state);

bool menu_nav_move_up(menu_nav_state_t *state);
bool menu_nav_move_down(menu_nav_state_t *state);
bool menu_nav_can_pop(const menu_nav_state_t *state);
bool menu_nav_pop(menu_nav_state_t *state);

bool menu_nav_enter(menu_nav_state_t *state,
                    menu_entry_type_t *entry_type,
                    const menu_page_t **submenu,
                    const menu_setting_desc_t **setting);

const menu_page_t *menu_nav_get_current_page(const menu_nav_state_t *state);
const menu_item_t *menu_nav_get_selected_item(const menu_nav_state_t *state);

uint16_t menu_nav_get_selected_index(const menu_nav_state_t *state);
uint16_t menu_nav_get_page_start(const menu_nav_state_t *state);
uint16_t menu_nav_get_total_pages(const menu_nav_state_t *state);
uint16_t menu_nav_get_current_page_index(const menu_nav_state_t *state);

uint16_t menu_nav_get_visible_items(const menu_nav_state_t *state,
                                    const menu_item_t **items,
                                    uint16_t max_items,
                                    uint16_t *first_index);

#endif /* MENU_NAV_H */
