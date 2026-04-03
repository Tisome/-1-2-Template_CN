#include "menu_data.h"

#include "data.h"
#include "menu_strings.h"

static const menu_setting_desc_t s_menu_setting_pipe_od = {
    MENU_SETTING_ID_PIPE_OD,
    MENU_TXT_PIPE_OD,
    MENU_SETTING_KIND_NUMERIC,
    "NUMERIC EDIT PAGE",
    {3U, 1U, "mm", 0.0, 999.9},
    {NULL, 0U}};

static const menu_setting_desc_t s_menu_setting_pipe_wall = {
    MENU_SETTING_ID_PIPE_WALL,
    MENU_TXT_PIPE_WALL,
    MENU_SETTING_KIND_NUMERIC,
    "NUMERIC EDIT PAGE",
    {2U, 1U, "mm", 0.0, 99.9},
    {NULL, 0U}};

static const menu_setting_option_t s_menu_pipe_material_options[] = {
    {"PVC", (uint32_t)PIPE_PVC},
    {"STEEL", (uint32_t)PIPE_METAL},
    {"ALLOY", (uint32_t)PIPE_ALLOY},
};

static const menu_setting_desc_t s_menu_setting_pipe_material = {
    MENU_SETTING_ID_PIPE_MATERIAL,
    MENU_TXT_PIPE_MATERIAL,
    MENU_SETTING_KIND_OPTION,
    "",
    {0U, 0U, NULL, 0.0, 0.0},
    {s_menu_pipe_material_options, (uint16_t)(sizeof(s_menu_pipe_material_options) / sizeof(s_menu_pipe_material_options[0]))}};

static const menu_setting_option_t s_menu_output_mode_options[] = {
    {"MODE0", 0U},
    {"MODE1", 1U},
    {"MODE2", 2U},
};

static const menu_setting_desc_t s_menu_setting_output_mode = {
    MENU_SETTING_ID_OUTPUT_MODE,
    MENU_TXT_OUTPUT_MODE,
    MENU_SETTING_KIND_OPTION,
    "",
    {0U, 0U, NULL, 0.0, 0.0},
    {s_menu_output_mode_options, (uint16_t)(sizeof(s_menu_output_mode_options) / sizeof(s_menu_output_mode_options[0]))}};

static const menu_setting_option_t s_menu_flow_unit_options[] = {
    {"m3/h", (uint32_t)RATE_UNIT_M3_P_H},
    {"m3/min", (uint32_t)RATE_UNIT_M3_P_MIN},
    {"m3/s", (uint32_t)RATE_UNIT_M3_P_S},
    {"L/h", (uint32_t)RATE_UNIT_L_P_H},
    {"L/min", (uint32_t)RATE_UNIT_L_P_MIN},
    {"L/s", (uint32_t)RATE_UNIT_L_P_S},
};

static const menu_setting_desc_t s_menu_setting_flow_unit = {
    MENU_SETTING_ID_FLOW_UNIT,
    MENU_TXT_FLOW_UNIT,
    MENU_SETTING_KIND_OPTION,
    "",
    {0U, 0U, NULL, 0.0, 0.0},
    {s_menu_flow_unit_options, (uint16_t)(sizeof(s_menu_flow_unit_options) / sizeof(s_menu_flow_unit_options[0]))}};

static const menu_setting_option_t s_menu_refresh_rate_options[] = {
    {"1", 1U},
    {"2", 2U},
    {"5", 5U},
    {"10", 10U},
};

static const menu_setting_desc_t s_menu_setting_refresh_rate = {
    MENU_SETTING_ID_REFRESH_RATE,
    MENU_TXT_REFRESH_RATE,
    MENU_SETTING_KIND_OPTION,
    "",
    {0U, 0U, NULL, 0.0, 0.0},
    {s_menu_refresh_rate_options, (uint16_t)(sizeof(s_menu_refresh_rate_options) / sizeof(s_menu_refresh_rate_options[0]))}};

static const menu_setting_desc_t s_menu_setting_alarm_low = {
    MENU_SETTING_ID_ALARM_LOW,
    MENU_TXT_ALARM_LOW,
    MENU_SETTING_KIND_NUMERIC,
    "NUMERIC EDIT PAGE",
    {4U, 1U, NULL, 0.0, 9999.9},
    {NULL, 0U}};

static const menu_setting_desc_t s_menu_setting_alarm_high = {
    MENU_SETTING_ID_ALARM_HIGH,
    MENU_TXT_ALARM_HIGH,
    MENU_SETTING_KIND_NUMERIC,
    "NUMERIC EDIT PAGE",
    {4U, 1U, NULL, 0.0, 9999.9},
    {NULL, 0U}};

static const menu_setting_desc_t s_menu_setting_zero_learn = {
    MENU_SETTING_ID_ZERO_LEARN,
    MENU_TXT_ZERO_LEARN,
    MENU_SETTING_KIND_ACTION,
    "OK RUN",
    {0U, 0U, NULL, 0.0, 0.0},
    {NULL, 0U}};

static const menu_setting_desc_t s_menu_setting_system = {
    MENU_SETTING_ID_SYSTEM,
    MENU_TXT_SYSTEM,
    MENU_SETTING_KIND_INFO,
    "",
    {0U, 0U, NULL, 0.0, 0.0},
    {NULL, 0U}};

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
