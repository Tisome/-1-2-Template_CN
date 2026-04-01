#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MENU_MAX_DEPTH 8U

typedef enum
{
    MENU_KEY_NONE = 0,
    MENU_KEY_UP,
    MENU_KEY_DOWN,
    MENU_KEY_OK,
    MENU_KEY_BACK
} menu_key_t;

typedef enum
{
    MENU_ACTION_STAY = 0,
    MENU_ACTION_BACK,
    MENU_ACTION_EXIT
} menu_action_result_t;

typedef menu_action_result_t (*menu_action_cb_t)(uint16_t selected_index, void *user_data);
typedef uint16_t (*menu_index_cb_t)(void *user_data);

struct menu;

typedef struct
{
    const char *label;
    const char *hint;
    const struct menu *child;
    menu_action_cb_t action;
    void *user_data;
} menu_item_t;

typedef struct menu
{
    const char *title;
    const menu_item_t *items;
    uint16_t item_count;
    uint16_t items_per_page;
    uint16_t default_index;
    bool option_menu;
    menu_index_cb_t get_current_index;
    void *user_data;
} menu_t;

typedef struct
{
    const menu_t *menu;
    uint16_t selected_index;
    uint16_t top_index;
} menu_frame_t;

typedef struct
{
    menu_frame_t frames[MENU_MAX_DEPTH];
    uint16_t depth;
    bool exit_requested;
} menu_state_t;

void menu_state_init(menu_state_t *state, const menu_t *root_menu);
bool menu_process_key(menu_state_t *state, menu_key_t key);

bool menu_is_exit_requested(const menu_state_t *state);
void menu_clear_exit_request(menu_state_t *state);

const menu_t *menu_get_current_menu(const menu_state_t *state);
const menu_item_t *menu_get_selected_item(const menu_state_t *state);
const menu_item_t *menu_get_item(const menu_state_t *state, uint16_t absolute_index);

uint16_t menu_get_selected_index(const menu_state_t *state);
uint16_t menu_get_top_index(const menu_state_t *state);
uint16_t menu_get_item_count(const menu_state_t *state);
uint16_t menu_get_items_per_page(const menu_state_t *state);

bool menu_current_menu_is_option(const menu_state_t *state);
bool menu_is_item_selected(const menu_state_t *state, uint16_t absolute_index);

uint16_t menu_get_visible_items(const menu_state_t *state,
                                const menu_item_t **items,
                                uint16_t max_items,
                                uint16_t *first_index);

#ifdef __cplusplus
}
#endif

#endif /* MENU_H */
