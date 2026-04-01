#include "menu.h"

#include <stddef.h>

static uint16_t menu_resolve_page_size(const menu_t *menu)
{
    if ((menu == NULL) || (menu->items_per_page == 0U))
    {
        return 1U;
    }

    return menu->items_per_page;
}

static uint16_t menu_resolve_selected_index(const menu_t *menu)
{
    uint16_t index = 0U;

    if ((menu == NULL) || (menu->item_count == 0U))
    {
        return 0U;
    }

    if (menu->get_current_index != NULL)
    {
        index = menu->get_current_index(menu->user_data);
    }
    else
    {
        index = menu->default_index;
    }

    if (index >= menu->item_count)
    {
        index = menu->item_count - 1U;
    }

    return index;
}

static uint16_t menu_resolve_top_index(const menu_t *menu, uint16_t selected_index)
{
    uint16_t page_size;

    if ((menu == NULL) || (menu->item_count == 0U))
    {
        return 0U;
    }

    page_size = menu_resolve_page_size(menu);

    if (menu->item_count <= page_size)
    {
        return 0U;
    }

    if ((selected_index + 1U) > page_size)
    {
        uint16_t top_index = (uint16_t)(selected_index + 1U - page_size);

        if ((top_index + page_size) > menu->item_count)
        {
            top_index = (uint16_t)(menu->item_count - page_size);
        }

        return top_index;
    }

    return 0U;
}

static void menu_make_frame(menu_frame_t *frame, const menu_t *menu)
{
    uint16_t selected_index;

    if (frame == NULL)
    {
        return;
    }

    frame->menu = menu;
    selected_index = menu_resolve_selected_index(menu);
    frame->selected_index = selected_index;
    frame->top_index = menu_resolve_top_index(menu, selected_index);
}

static menu_frame_t *menu_current_frame(menu_state_t *state)
{
    if ((state == NULL) || (state->depth == 0U))
    {
        return NULL;
    }

    return &state->frames[state->depth - 1U];
}

static const menu_frame_t *menu_current_frame_const(const menu_state_t *state)
{
    if ((state == NULL) || (state->depth == 0U))
    {
        return NULL;
    }

    return &state->frames[state->depth - 1U];
}

static void menu_keep_selection_visible(menu_frame_t *frame)
{
    uint16_t page_size;

    if ((frame == NULL) || (frame->menu == NULL) || (frame->menu->item_count == 0U))
    {
        return;
    }

    page_size = menu_resolve_page_size(frame->menu);

    if (frame->selected_index < frame->top_index)
    {
        frame->top_index = frame->selected_index;
    }
    else if (frame->selected_index >= (frame->top_index + page_size))
    {
        frame->top_index = (uint16_t)(frame->selected_index + 1U - page_size);
    }
}

void menu_state_init(menu_state_t *state, const menu_t *root_menu)
{
    if (state == NULL)
    {
        return;
    }

    state->depth = 0U;
    state->exit_requested = false;

    if (root_menu != NULL)
    {
        menu_make_frame(&state->frames[0], root_menu);
        state->depth = 1U;
    }
}

bool menu_process_key(menu_state_t *state, menu_key_t key)
{
    menu_frame_t *frame = menu_current_frame(state);

    if ((frame == NULL) || (frame->menu == NULL))
    {
        return false;
    }

    switch (key)
    {
    case MENU_KEY_UP:
        if (frame->selected_index > 0U)
        {
            frame->selected_index--;
            menu_keep_selection_visible(frame);
            return true;
        }
        return false;

    case MENU_KEY_DOWN:
        if ((frame->menu->item_count > 0U) && (frame->selected_index + 1U < frame->menu->item_count))
        {
            frame->selected_index++;
            menu_keep_selection_visible(frame);
            return true;
        }
        return false;

    case MENU_KEY_OK:
        if ((frame->menu->item_count == 0U) || (frame->selected_index >= frame->menu->item_count))
        {
            return false;
        }
        else
        {
            const menu_item_t *item = &frame->menu->items[frame->selected_index];

            if (item->child != NULL)
            {
                if (state->depth < MENU_MAX_DEPTH)
                {
                    menu_make_frame(&state->frames[state->depth], item->child);
                    state->depth++;
                    return true;
                }

                return false;
            }

            if (item->action != NULL)
            {
                menu_action_result_t result = item->action(frame->selected_index, item->user_data);

                if (result == MENU_ACTION_BACK)
                {
                    if (state->depth > 1U)
                    {
                        state->depth--;
                    }
                    else
                    {
                        state->exit_requested = true;
                    }
                }
                else if (result == MENU_ACTION_EXIT)
                {
                    state->exit_requested = true;
                }

                return true;
            }
        }
        return false;

    case MENU_KEY_BACK:
        if (state->depth > 1U)
        {
            state->depth--;
        }
        else
        {
            state->exit_requested = true;
        }
        return true;

    case MENU_KEY_NONE:
    default:
        return false;
    }
}

bool menu_is_exit_requested(const menu_state_t *state)
{
    if (state == NULL)
    {
        return false;
    }

    return state->exit_requested;
}

void menu_clear_exit_request(menu_state_t *state)
{
    if (state == NULL)
    {
        return;
    }

    state->exit_requested = false;
}

const menu_t *menu_get_current_menu(const menu_state_t *state)
{
    const menu_frame_t *frame = menu_current_frame_const(state);

    if (frame == NULL)
    {
        return NULL;
    }

    return frame->menu;
}

const menu_item_t *menu_get_selected_item(const menu_state_t *state)
{
    const menu_frame_t *frame = menu_current_frame_const(state);

    if ((frame == NULL) || (frame->menu == NULL) || (frame->menu->item_count == 0U))
    {
        return NULL;
    }

    return &frame->menu->items[frame->selected_index];
}

const menu_item_t *menu_get_item(const menu_state_t *state, uint16_t absolute_index)
{
    const menu_frame_t *frame = menu_current_frame_const(state);

    if ((frame == NULL) || (frame->menu == NULL) || (absolute_index >= frame->menu->item_count))
    {
        return NULL;
    }

    return &frame->menu->items[absolute_index];
}

uint16_t menu_get_selected_index(const menu_state_t *state)
{
    const menu_frame_t *frame = menu_current_frame_const(state);

    if (frame == NULL)
    {
        return 0U;
    }

    return frame->selected_index;
}

uint16_t menu_get_top_index(const menu_state_t *state)
{
    const menu_frame_t *frame = menu_current_frame_const(state);

    if (frame == NULL)
    {
        return 0U;
    }

    return frame->top_index;
}

uint16_t menu_get_item_count(const menu_state_t *state)
{
    const menu_frame_t *frame = menu_current_frame_const(state);

    if ((frame == NULL) || (frame->menu == NULL))
    {
        return 0U;
    }

    return frame->menu->item_count;
}

uint16_t menu_get_items_per_page(const menu_state_t *state)
{
    const menu_frame_t *frame = menu_current_frame_const(state);

    if ((frame == NULL) || (frame->menu == NULL))
    {
        return 0U;
    }

    return menu_resolve_page_size(frame->menu);
}

bool menu_current_menu_is_option(const menu_state_t *state)
{
    const menu_frame_t *frame = menu_current_frame_const(state);

    if ((frame == NULL) || (frame->menu == NULL))
    {
        return false;
    }

    return frame->menu->option_menu;
}

bool menu_is_item_selected(const menu_state_t *state, uint16_t absolute_index)
{
    const menu_frame_t *frame = menu_current_frame_const(state);

    if ((frame == NULL) || (frame->menu == NULL))
    {
        return false;
    }

    return frame->selected_index == absolute_index;
}

uint16_t menu_get_visible_items(const menu_state_t *state,
                                const menu_item_t **items,
                                uint16_t max_items,
                                uint16_t *first_index)
{
    const menu_frame_t *frame = menu_current_frame_const(state);
    uint16_t page_size;
    uint16_t count;
    uint16_t i;

    if ((frame == NULL) || (frame->menu == NULL))
    {
        if (first_index != NULL)
        {
            *first_index = 0U;
        }
        return 0U;
    }

    if (first_index != NULL)
    {
        *first_index = frame->top_index;
    }

    page_size = menu_resolve_page_size(frame->menu);
    count = (uint16_t)(frame->menu->item_count - frame->top_index);

    if (count > page_size)
    {
        count = page_size;
    }

    if (count > max_items)
    {
        count = max_items;
    }

    for (i = 0U; i < count; i++)
    {
        items[i] = &frame->menu->items[frame->top_index + i];
    }

    return count;
}
