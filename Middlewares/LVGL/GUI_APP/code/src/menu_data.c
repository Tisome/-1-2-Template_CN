#include "menu_data.h"

#include "menu_strings.h"

static const menu_setting_desc_t s_menu_setting_pipe_od = {
    MENU_TXT_PIPE_OD,
    MENU_TXT_PLACEHOLDER};

static const menu_setting_desc_t s_menu_setting_pipe_wall = {
    MENU_TXT_PIPE_WALL,
    MENU_TXT_PLACEHOLDER};

static const menu_setting_desc_t s_menu_setting_pipe_material = {
    MENU_TXT_PIPE_MATERIAL,
    MENU_TXT_PLACEHOLDER};

static const menu_setting_desc_t s_menu_setting_output_mode = {
    MENU_TXT_OUTPUT_MODE,
    MENU_TXT_PLACEHOLDER};

static const menu_setting_desc_t s_menu_setting_flow_unit = {
    MENU_TXT_FLOW_UNIT,
    MENU_TXT_PLACEHOLDER};

static const menu_setting_desc_t s_menu_setting_refresh_rate = {
    MENU_TXT_REFRESH_RATE,
    MENU_TXT_PLACEHOLDER};

static const menu_setting_desc_t s_menu_setting_alarm_low = {
    MENU_TXT_ALARM_LOW,
    MENU_TXT_PLACEHOLDER};

static const menu_setting_desc_t s_menu_setting_alarm_high = {
    MENU_TXT_ALARM_HIGH,
    MENU_TXT_PLACEHOLDER};

static const menu_setting_desc_t s_menu_setting_zero_learn = {
    MENU_TXT_ZERO_LEARN,
    MENU_TXT_PLACEHOLDER};

static const menu_setting_desc_t s_menu_setting_system = {
    MENU_TXT_SYSTEM,
    MENU_TXT_PLACEHOLDER};

static const menu_page_t s_menu_pipe_page;
static const menu_page_t s_menu_output_page;
static const menu_page_t s_menu_alarm_page;

static const menu_item_t s_menu_pipe_items[] = {
    {MENU_TXT_PIPE_OD, MENU_ENTRY_SETTING, NULL, &s_menu_setting_pipe_od},
    {MENU_TXT_PIPE_WALL, MENU_ENTRY_SETTING, NULL, &s_menu_setting_pipe_wall},
    {MENU_TXT_PIPE_MATERIAL, MENU_ENTRY_SETTING, NULL, &s_menu_setting_pipe_material},
};

static const menu_item_t s_menu_output_items[] = {
    {MENU_TXT_OUTPUT_MODE, MENU_ENTRY_SETTING, NULL, &s_menu_setting_output_mode},
    {MENU_TXT_FLOW_UNIT, MENU_ENTRY_SETTING, NULL, &s_menu_setting_flow_unit},
    {MENU_TXT_REFRESH_RATE, MENU_ENTRY_SETTING, NULL, &s_menu_setting_refresh_rate},
};

static const menu_item_t s_menu_alarm_items[] = {
    {MENU_TXT_ALARM_LOW, MENU_ENTRY_SETTING, NULL, &s_menu_setting_alarm_low},
    {MENU_TXT_ALARM_HIGH, MENU_ENTRY_SETTING, NULL, &s_menu_setting_alarm_high},
    {MENU_TXT_ZERO_LEARN, MENU_ENTRY_SETTING, NULL, &s_menu_setting_zero_learn},
};

static const menu_page_t s_menu_pipe_page = {
    MENU_TXT_PIPE,
    s_menu_pipe_items,
    (uint16_t)(sizeof(s_menu_pipe_items) / sizeof(s_menu_pipe_items[0])),
    MENU_VISIBLE_ITEMS,
    0U};

static const menu_page_t s_menu_output_page = {
    MENU_TXT_OUTPUT,
    s_menu_output_items,
    (uint16_t)(sizeof(s_menu_output_items) / sizeof(s_menu_output_items[0])),
    MENU_VISIBLE_ITEMS,
    0U};

static const menu_page_t s_menu_alarm_page = {
    MENU_TXT_ALARM,
    s_menu_alarm_items,
    (uint16_t)(sizeof(s_menu_alarm_items) / sizeof(s_menu_alarm_items[0])),
    MENU_VISIBLE_ITEMS,
    0U};

static const menu_item_t s_menu_root_items[] = {
    {MENU_TXT_MEASURE, MENU_ENTRY_MEASURE, NULL, NULL},
    {MENU_TXT_PIPE, MENU_ENTRY_SUBMENU, &s_menu_pipe_page, NULL},
    {MENU_TXT_OUTPUT, MENU_ENTRY_SUBMENU, &s_menu_output_page, NULL},
    {MENU_TXT_ALARM, MENU_ENTRY_SUBMENU, &s_menu_alarm_page, NULL},
    {MENU_TXT_SYSTEM, MENU_ENTRY_SETTING, NULL, &s_menu_setting_system},
};

static const menu_page_t s_menu_root_page = {
    MENU_TXT_TITLE,
    s_menu_root_items,
    (uint16_t)(sizeof(s_menu_root_items) / sizeof(s_menu_root_items[0])),
    MENU_VISIBLE_ITEMS,
    0U};

const menu_page_t *menu_data_get_root_page(void)
{
    return &s_menu_root_page;
}
