/*
 * 菜单导航状态机文件。
 * 它只处理“当前在哪一页、选中了哪一项、是否进入子菜单、是否返回上一层”，
 * 不直接操作 LVGL 控件，是界面渲染层和菜单数据层之间的纯逻辑层。
 */
#include "menu_nav.h"

/* 解析页面每页显示项数，未配置时使用默认可见数量。 */
static uint16_t menu_nav_resolve_page_size(const menu_page_t *page)
{
    if ((page == NULL) || (page->items_per_page == 0U))
    {
        return MENU_VISIBLE_ITEMS;
    }

    return page->items_per_page;
}

/* 解析页面默认选中项，并保证结果不会越界。 */
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

/* 用指定页面填充一个导航栈帧。 */
static void menu_nav_set_frame(menu_nav_frame_t *frame, const menu_page_t *page)
{
    if (frame == NULL)
    {
        return;
    }

    frame->page = page;
    frame->selected_index = menu_nav_resolve_default_index(page);
}

/* 获取当前可写导航帧。 */
static menu_nav_frame_t *menu_nav_get_current_frame(menu_nav_state_t *state)
{
    if ((state == NULL) || (state->depth == 0U))
    {
        return NULL;
    }

    return &state->frames[state->depth - 1U];
}

/* 获取当前只读导航帧。 */
static const menu_nav_frame_t *menu_nav_get_current_frame_const(const menu_nav_state_t *state)
{
    if ((state == NULL) || (state->depth == 0U))
    {
        return NULL;
    }

    return &state->frames[state->depth - 1U];
}

/* 初始化导航状态，并把根页面压入导航栈。 */
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

/* 将导航状态重置回根页面。 */
void menu_nav_reset_root(menu_nav_state_t *state, const menu_page_t *root_page)
{
    menu_nav_init(state, root_page);
}

/* 恢复当前页面的默认选中项。 */
void menu_nav_reset_current_selection(menu_nav_state_t *state)
{
    menu_nav_frame_t *frame = menu_nav_get_current_frame(state);

    if (frame == NULL)
    {
        return;
    }

    frame->selected_index = menu_nav_resolve_default_index(frame->page);
}

/* 在当前页面内向上移动选中项。 */
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

/* 在当前页面内向下移动选中项。 */
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

/* 判断当前是否还能返回上一层菜单。 */
bool menu_nav_can_pop(const menu_nav_state_t *state)
{
    return (state != NULL) && (state->depth > 1U);
}

/* 从当前页面返回上一层页面。 */
bool menu_nav_pop(menu_nav_state_t *state)
{
    if (!menu_nav_can_pop(state))
    {
        return false;
    }

    state->depth--;
    return true;
}

/*
 * 进入当前选中项。
 * 如果是子菜单，会把子页面压入导航栈；
 * 如果是设置项或测量页，则把对应元数据返回给上层进一步处理。
 */
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

/* 获取当前所在页面。 */
const menu_page_t *menu_nav_get_current_page(const menu_nav_state_t *state)
{
    const menu_nav_frame_t *frame = menu_nav_get_current_frame_const(state);

    if (frame == NULL)
    {
        return NULL;
    }

    return frame->page;
}

/* 获取当前选中的菜单项。 */
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

/* 获取当前选中项在本页中的索引。 */
uint16_t menu_nav_get_selected_index(const menu_nav_state_t *state)
{
    const menu_nav_frame_t *frame = menu_nav_get_current_frame_const(state);

    if (frame == NULL)
    {
        return 0U;
    }

    return frame->selected_index;
}

/* 计算当前页在整张菜单表中的起始索引。 */
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

/* 计算当前页面总页数。 */
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

/* 计算当前选中项位于第几页，页号从 1 开始。 */
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

/* 取出当前页需要显示的菜单项列表，供列表页批量渲染。 */
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
