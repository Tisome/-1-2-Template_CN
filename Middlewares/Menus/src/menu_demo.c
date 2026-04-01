#include "menu_demo.h"

#include "bsp_key.h"
#include "data.h"

#include <stddef.h>

static menu_action_result_t menu_demo_set_pipe_type(uint16_t selected_index, void *user_data);
static menu_action_result_t menu_demo_set_speed_unit(uint16_t selected_index, void *user_data);
static menu_action_result_t menu_demo_set_rate_unit(uint16_t selected_index, void *user_data);
static menu_action_result_t menu_demo_set_display_rate(uint16_t selected_index, void *user_data);
static menu_action_result_t menu_demo_save_parameters(uint16_t selected_index, void *user_data);

static uint16_t menu_demo_get_pipe_type(void *user_data);
static uint16_t menu_demo_get_speed_unit(void *user_data);
static uint16_t menu_demo_get_rate_unit(void *user_data);
static uint16_t menu_demo_get_display_rate(void *user_data);

static const menu_item_t s_pipe_type_items[] = {
    {"PVC", "plastic pipe", NULL, menu_demo_set_pipe_type, NULL},
    {"METAL", "metal pipe", NULL, menu_demo_set_pipe_type, NULL},
    {"ALLOY", "alloy pipe", NULL, menu_demo_set_pipe_type, NULL},
};

static const menu_item_t s_speed_unit_items[] = {
    {"m/s", "meter per second", NULL, menu_demo_set_speed_unit, NULL},
    {"cm/s", "centimeter per second", NULL, menu_demo_set_speed_unit, NULL},
    {"mm/s", "millimeter per second", NULL, menu_demo_set_speed_unit, NULL},
};

static const menu_item_t s_rate_unit_items[] = {
    {"m3/h", "cubic meter per hour", NULL, menu_demo_set_rate_unit, NULL},
    {"L/min", "liter per minute", NULL, menu_demo_set_rate_unit, NULL},
    {"L/s", "liter per second", NULL, menu_demo_set_rate_unit, NULL},
};

static const menu_item_t s_display_rate_items[] = {
    {"1 Hz", "refresh once each second", NULL, menu_demo_set_display_rate, NULL},
    {"5 Hz", "refresh five times each second", NULL, menu_demo_set_display_rate, NULL},
    {"10 Hz", "refresh ten times each second", NULL, menu_demo_set_display_rate, NULL},
};

static const menu_t s_pipe_type_menu = {
    "Pipe Type",
    s_pipe_type_items,
    (uint16_t)(sizeof(s_pipe_type_items) / sizeof(s_pipe_type_items[0])),
    4U,
    0U,
    true,
    menu_demo_get_pipe_type,
    NULL};

static const menu_t s_speed_unit_menu = {
    "Speed Unit",
    s_speed_unit_items,
    (uint16_t)(sizeof(s_speed_unit_items) / sizeof(s_speed_unit_items[0])),
    4U,
    0U,
    true,
    menu_demo_get_speed_unit,
    NULL};

static const menu_t s_rate_unit_menu = {
    "Flow Rate Unit",
    s_rate_unit_items,
    (uint16_t)(sizeof(s_rate_unit_items) / sizeof(s_rate_unit_items[0])),
    4U,
    0U,
    true,
    menu_demo_get_rate_unit,
    NULL};

static const menu_t s_display_rate_menu = {
    "Refresh Rate",
    s_display_rate_items,
    (uint16_t)(sizeof(s_display_rate_items) / sizeof(s_display_rate_items[0])),
    4U,
    0U,
    true,
    menu_demo_get_display_rate,
    NULL};

static const menu_item_t s_root_items[] = {
    {"Pipe Type", "choose pipe material", &s_pipe_type_menu, NULL, NULL},
    {"Speed Unit", "choose displayed speed unit", &s_speed_unit_menu, NULL, NULL},
    {"Flow Rate Unit", "choose displayed rate unit", &s_rate_unit_menu, NULL, NULL},
    {"Refresh Rate", "choose display update speed", &s_display_rate_menu, NULL, NULL},
    {"Save Parameters", "store current values into eeprom", NULL, menu_demo_save_parameters, NULL},
};

static const menu_t s_root_menu = {
    "Menu Demo",
    s_root_items,
    (uint16_t)(sizeof(s_root_items) / sizeof(s_root_items[0])),
    4U,
    0U,
    false,
    NULL,
    NULL};

static menu_action_result_t menu_demo_set_pipe_type(uint16_t selected_index, void *user_data)
{
    (void)user_data;

    switch (selected_index)
    {
    case 0U:
        g_parameters.pipe_type = PIPE_PVC;
        break;
    case 1U:
        g_parameters.pipe_type = PIPE_METAL;
        break;
    case 2U:
    default:
        g_parameters.pipe_type = PIPE_ALLOY;
        break;
    }

    g_parameters.is_saved = 1U;
    return MENU_ACTION_BACK;
}

static menu_action_result_t menu_demo_set_speed_unit(uint16_t selected_index, void *user_data)
{
    (void)user_data;

    switch (selected_index)
    {
    case 0U:
        g_parameters.speed_unit_type = SPEED_UNIT_M_P_S;
        break;
    case 1U:
        g_parameters.speed_unit_type = SPEED_UNIT_CM_P_S;
        break;
    case 2U:
    default:
        g_parameters.speed_unit_type = SPEED_UNIT_MM_P_S;
        break;
    }

    g_parameters.is_saved = 1U;
    return MENU_ACTION_BACK;
}

static menu_action_result_t menu_demo_set_rate_unit(uint16_t selected_index, void *user_data)
{
    (void)user_data;

    switch (selected_index)
    {
    case 0U:
        g_parameters.rate_unit_type = RATE_UNIT_M3_P_H;
        break;
    case 1U:
        g_parameters.rate_unit_type = RATE_UNIT_L_P_MIN;
        break;
    case 2U:
    default:
        g_parameters.rate_unit_type = RATE_UNIT_L_P_S;
        break;
    }

    g_parameters.is_saved = 1U;
    return MENU_ACTION_BACK;
}

static menu_action_result_t menu_demo_set_display_rate(uint16_t selected_index, void *user_data)
{
    (void)user_data;

    switch (selected_index)
    {
    case 0U:
        g_parameters.display_sensitivity = 1U;
        break;
    case 1U:
        g_parameters.display_sensitivity = 5U;
        break;
    case 2U:
    default:
        g_parameters.display_sensitivity = 10U;
        break;
    }

    g_parameters.is_saved = 1U;
    return MENU_ACTION_BACK;
}

static menu_action_result_t menu_demo_save_parameters(uint16_t selected_index, void *user_data)
{
    (void)selected_index;
    (void)user_data;

    g_parameters.is_saved = 1U;
    (void)SaveParameters(&g_parameters);

    return MENU_ACTION_STAY;
}

static uint16_t menu_demo_get_pipe_type(void *user_data)
{
    (void)user_data;
    return (uint16_t)g_parameters.pipe_type;
}

static uint16_t menu_demo_get_speed_unit(void *user_data)
{
    (void)user_data;
    return (uint16_t)g_parameters.speed_unit_type;
}

static uint16_t menu_demo_get_rate_unit(void *user_data)
{
    (void)user_data;

    switch (g_parameters.rate_unit_type)
    {
    case RATE_UNIT_M3_P_H:
        return 0U;
    case RATE_UNIT_L_P_MIN:
        return 1U;
    case RATE_UNIT_L_P_S:
        return 2U;
    default:
        return 0U;
    }
}

static uint16_t menu_demo_get_display_rate(void *user_data)
{
    (void)user_data;

    switch (g_parameters.display_sensitivity)
    {
    case 1U:
        return 0U;
    case 5U:
        return 1U;
    case 10U:
        return 2U;
    default:
        return 1U;
    }
}

const menu_t *menu_demo_get_root(void)
{
    return &s_root_menu;
}

menu_key_t menu_demo_map_key(uint8_t key_value)
{
    switch (key_value)
    {
    case KEY1_PRESS:
        return MENU_KEY_UP;
    case KEY2_PRESS:
        return MENU_KEY_DOWN;
    case KEY3_PRESS:
        return MENU_KEY_OK;
    case KEY4_PRESS:
    case KEY4_LONG_PRESS:
        return MENU_KEY_BACK;
    default:
        return MENU_KEY_NONE;
    }
}
