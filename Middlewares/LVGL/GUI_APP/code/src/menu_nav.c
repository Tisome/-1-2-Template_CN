#include "menu_nav.h"

static uint16_t menu_nav_resolve_page_size(const menu_page_t *page)
{
    if ((page == NULL) || (page->items_per_page == 0U))
    {
        return MENU_VISIBLE_ITEMS;
    }

    return page->items_per_page;
}

static uint16_t menu_nav_resolve_default_index(const menu_page_t *page)
{
    uint16_t index = 0U;

    if ((page == NULL) || (page->item_count == 0U))
    {
        return 0U;
    }

    index = page->default_index;
    if (index >= page->item_count)
    {
        index = (uint16_t)(page->item_count - 1U);
    }

    return index;
}

static void menu_nav_set_frame(menu_nav_frame_t *frame, const menu_page_t *page)
{
    if (frame == NULL)
    {
        return;
    }

    frame->page = page;
    frame->selected_index = menu_nav_resolve_default_index(page);
}

static menu_nav_frame_t *menu_nav_get_current_frame(menu_nav_state_t *state)
{
    if ((state == NULL) || (state->depth == 0U))
    {
        return NULL;
    }

    return &state->frames[state->depth - 1U];
}

static const menu_nav_frame_t *menu_nav_get_current_frame_const(const menu_nav_state_t *state)
{
    if ((state == NULL) || (state->depth == 0U))
    {
        return NULL;
    }

    return &state->frames[state->depth - 1U];
}

void menu_nav_init(menu_nav_state_t *state, const menu_page_t *root_page)
{
    if (state == NULL)
    {
        return;
    }

    state->depth = 0U;

    if (root_page != NULL)
    {
        menu_nav_set_frame(&state->frames[0], root_page);
        state->depth = 1U;
    }
}

void menu_nav_reset_root(menu_nav_state_t *state, const menu_page_t *root_page)
{
    menu_nav_init(state, root_page);
}

void menu_nav_reset_current_selection(menu_nav_state_t *state)
{
    menu_nav_frame_t *frame = menu_nav_get_current_frame(state);

    if (frame == NULL)
    {
        return;
    }

    frame->selected_index = menu_nav_resolve_default_index(frame->page);
}

bool menu_nav_move_up(menu_nav_state_t *state)
{
    menu_nav_frame_t *frame = menu_nav_get_current_frame(state);

    if ((frame == NULL) || (frame->page == NULL) || (frame->selected_index == 0U))
    {
        return false;
    }

    frame->selected_index--;
    return true;
}

bool menu_nav_move_down(menu_nav_state_t *state)
{
    menu_nav_frame_t *frame = menu_nav_get_current_frame(state);

    if ((frame == NULL) || (frame->page == NULL) || (frame->page->item_count == 0U))
    {
        return false;
    }

    if ((frame->selected_index + 1U) >= frame->page->item_count)
    {
        return false;
    }

    frame->selected_index++;
    return true;
}

bool menu_nav_can_pop(const menu_nav_state_t *state)
{
    return (state != NULL) && (state->depth > 1U);
}

bool menu_nav_pop(menu_nav_state_t *state)
{
    if (!menu_nav_can_pop(state))
    {
        return false;
    }

    state->depth--;
    return true;
}

bool menu_nav_enter(menu_nav_state_t *state,
                    menu_entry_type_t *entry_type,
                    const menu_page_t **submenu,
                    const menu_setting_desc_t **setting)
{
    menu_nav_frame_t *frame = menu_nav_get_current_frame(state);
    const menu_item_t *item;

    if ((frame == NULL) || (frame->page == NULL) || (frame->page->item_count == 0U))
    {
        return false;
    }

    if (frame->selected_index >= frame->page->item_count)
    {
        return false;
    }

    item = &frame->page->items[frame->selected_index];

    if (entry_type != NULL)
    {
        *entry_type = item->type;
    }

    if (submenu != NULL)
    {
        *submenu = item->submenu;
    }

    if (setting != NULL)
    {
        *setting = item->setting;
    }

    if ((item->type == MENU_ENTRY_SUBMENU) && (item->submenu != NULL))
    {
        if (state->depth >= MENU_NAV_MAX_DEPTH)
        {
            return false;
        }

        menu_nav_set_frame(&state->frames[state->depth], item->submenu);
        state->depth++;
    }

    return true;
}

const menu_page_t *menu_nav_get_current_page(const menu_nav_state_t *state)
{
    const menu_nav_frame_t *frame = menu_nav_get_current_frame_const(state);

    if (frame == NULL)
    {
        return NULL;
    }

    return frame->page;
}

const menu_item_t *menu_nav_get_selected_item(const menu_nav_state_t *state)
{
    const menu_nav_frame_t *frame = menu_nav_get_current_frame_const(state);

    if ((frame == NULL) || (frame->page == NULL) || (frame->page->item_count == 0U))
    {
        return NULL;
    }

    if (frame->selected_index >= frame->page->item_count)
    {
        return NULL;
    }

    return &frame->page->items[frame->selected_index];
}

uint16_t menu_nav_get_selected_index(const menu_nav_state_t *state)
{
    const menu_nav_frame_t *frame = menu_nav_get_current_frame_const(state);

    if (frame == NULL)
    {
        return 0U;
    }

    return frame->selected_index;
}

uint16_t menu_nav_get_page_start(const menu_nav_state_t *state)
{
    const menu_nav_frame_t *frame = menu_nav_get_current_frame_const(state);
    uint16_t page_size;

    if ((frame == NULL) || (frame->page == NULL))
    {
        return 0U;
    }

    page_size = menu_nav_resolve_page_size(frame->page);
    return (uint16_t)((frame->selected_index / page_size) * page_size);
}

uint16_t menu_nav_get_total_pages(const menu_nav_state_t *state)
{
    const menu_page_t *page = menu_nav_get_current_page(state);
    uint16_t page_size;

    if ((page == NULL) || (page->item_count == 0U))
    {
        return 0U;
    }

    page_size = menu_nav_resolve_page_size(page);
    return (uint16_t)((page->item_count + page_size - 1U) / page_size);
}

uint16_t menu_nav_get_current_page_index(const menu_nav_state_t *state)
{
    const menu_nav_frame_t *frame = menu_nav_get_current_frame_const(state);
    uint16_t page_size;

    if ((frame == NULL) || (frame->page == NULL) || (frame->page->item_count == 0U))
    {
        return 0U;
    }

    page_size = menu_nav_resolve_page_size(frame->page);
    return (uint16_t)(frame->selected_index / page_size + 1U);
}

uint16_t menu_nav_get_visible_items(const menu_nav_state_t *state,
                                    const menu_item_t **items,
                                    uint16_t max_items,
                                    uint16_t *first_index)
{
    const menu_page_t *page = menu_nav_get_current_page(state);
    uint16_t page_size;
    uint16_t start_index;
    uint16_t count;
    uint16_t index;

    if ((page == NULL) || (page->item_count == 0U))
    {
        if (first_index != NULL)
        {
            *first_index = 0U;
        }

        return 0U;
    }

    page_size = menu_nav_resolve_page_size(page);
    start_index = menu_nav_get_page_start(state);
    count = (uint16_t)(page->item_count - start_index);

    if (count > page_size)
    {
        count = page_size;
    }

    if (count > max_items)
    {
        count = max_items;
    }

    if (first_index != NULL)
    {
        *first_index = start_index;
    }

    if (items != NULL)
    {
        for (index = 0U; index < count; index++)
        {
            items[index] = &page->items[start_index + index];
        }
    }

    return count;
}
