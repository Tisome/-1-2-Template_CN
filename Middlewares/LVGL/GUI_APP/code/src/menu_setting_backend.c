/*
 * 设置页后端适配文件。
 * 该文件负责把 GUI 中的设置项 ID 映射到参数子系统接口，
 * 让设置页只关心“有哪些项、怎么显示”，而具体值从哪里读写由这里统一处理。
 */
#include "menu_setting_backend.h"

#include <stdio.h>
#include <string.h>

/* 在选项列表中按值查找当前选中的选项描述。 */
static const menu_setting_option_t *menu_setting_backend_find_option(const menu_setting_desc_t *setting,
                                                                     uint32_t value)
{
    uint16_t index;

    if ((setting == NULL) ||
        (setting->option.options == NULL) ||
        (setting->option.option_count == 0U))
    {
        return NULL;
    }

    for (index = 0U; index < setting->option.option_count; index++)
    {
        if (setting->option.options[index].value == value)
        {
            return &setting->option.options[index];
        }
    }

    return NULL;
}

/* 安全复制详情文本，避免页面层直接处理字符串长度问题。 */
static void menu_setting_backend_copy_detail(menu_setting_view_t *view,
                                             const char *detail_text)
{
    if (view == NULL)
    {
        return;
    }

    if (detail_text == NULL)
    {
        view->detail_text[0] = '\0';
        return;
    }

    (void)snprintf(view->detail_text, sizeof(view->detail_text), "%s", detail_text);
}

/* 读取数值型设置项当前值，供设置页显示或编辑时使用。 */
bool menu_setting_backend_get_numeric_value(menu_setting_id_t id, double *value)
{
    switch (id)
    {
    case MENU_SETTING_ID_PIPE_OD:
        return parameter_get_double(PARAMETER_FIELD_INNER_DIAMETER, value);

    case MENU_SETTING_ID_PIPE_WALL:
        return parameter_get_double(PARAMETER_FIELD_WALL_THICK, value);

    case MENU_SETTING_ID_ALARM_LOW:
        return parameter_get_double(PARAMETER_FIELD_ALARM_LOWER_RATE_RANGE, value);

    case MENU_SETTING_ID_ALARM_HIGH:
        return parameter_get_double(PARAMETER_FIELD_ALARM_UPPER_RATE_RANGE, value);

    case MENU_SETTING_ID_ZERO_LEARN:
        return parameter_get_double(PARAMETER_FIELD_ZERO_OFFSET_SPEED, value);

    default:
        return false;
    }
}

/* 读取枚举/选项型设置项当前值。 */
bool menu_setting_backend_get_option_value(menu_setting_id_t id, uint32_t *value)
{
    switch (id)
    {
    case MENU_SETTING_ID_PIPE_MATERIAL:
        return parameter_get_u32(PARAMETER_FIELD_PIPE_TYPE, value);

    case MENU_SETTING_ID_OUTPUT_MODE:
        return parameter_get_u32(PARAMETER_FIELD_OUTPUT_MODE, value);

    case MENU_SETTING_ID_FLOW_UNIT:
        return parameter_get_u32(PARAMETER_FIELD_RATE_UNIT_TYPE, value);

    case MENU_SETTING_ID_REFRESH_RATE:
        return parameter_get_u32(PARAMETER_FIELD_DISPLAY_SENSITIVITY, value);

    default:
        return false;
    }
}

/* 提交一个数值型设置项到参数系统。 */
parameter_apply_status_t menu_setting_backend_commit_numeric(menu_setting_id_t id, double value)
{
    switch (id)
    {
    case MENU_SETTING_ID_PIPE_OD:
        return parameter_set_double(PARAMETER_FIELD_INNER_DIAMETER, value);

    case MENU_SETTING_ID_PIPE_WALL:
        return parameter_set_double(PARAMETER_FIELD_WALL_THICK, value);

    case MENU_SETTING_ID_ALARM_LOW:
        return parameter_set_double(PARAMETER_FIELD_ALARM_LOWER_RATE_RANGE, value);

    case MENU_SETTING_ID_ALARM_HIGH:
        return parameter_set_double(PARAMETER_FIELD_ALARM_UPPER_RATE_RANGE, value);

    default:
        return PARAMETER_APPLY_UNSUPPORTED;
    }
}

/* 提交一个选项型设置项到参数系统。 */
parameter_apply_status_t menu_setting_backend_commit_option(menu_setting_id_t id, uint32_t value)
{
    switch (id)
    {
    case MENU_SETTING_ID_PIPE_MATERIAL:
        return parameter_set_u32(PARAMETER_FIELD_PIPE_TYPE, value);

    case MENU_SETTING_ID_OUTPUT_MODE:
        return parameter_set_u32(PARAMETER_FIELD_OUTPUT_MODE, value);

    case MENU_SETTING_ID_FLOW_UNIT:
        return parameter_set_u32(PARAMETER_FIELD_RATE_UNIT_TYPE, value);

    case MENU_SETTING_ID_REFRESH_RATE:
        return parameter_set_u32(PARAMETER_FIELD_DISPLAY_SENSITIVITY, value);

    default:
        return PARAMETER_APPLY_UNSUPPORTED;
    }
}

/*
 * 让选项型设置项向上或向下切换一步。
 * 本函数负责把“步进操作”转换为新的选项值并提交，主要给设置页按键操作使用。
 */
parameter_apply_status_t menu_setting_backend_step_option(const menu_setting_desc_t *setting, int8_t step)
{
    uint32_t current_value = 0U;
    uint16_t current_index = 0U;
    int32_t next_index = 0;
    uint16_t index;

    if ((setting == NULL) ||
        (setting->kind != MENU_SETTING_KIND_OPTION) ||
        (setting->option.options == NULL) ||
        (setting->option.option_count == 0U))
    {
        return PARAMETER_APPLY_UNSUPPORTED;
    }

    if (!menu_setting_backend_get_option_value(setting->id, &current_value))
    {
        return PARAMETER_APPLY_UNSUPPORTED;
    }

    for (index = 0U; index < setting->option.option_count; index++)
    {
        if (setting->option.options[index].value == current_value)
        {
            current_index = index;
            break;
        }
    }

    next_index = (int32_t)current_index + (int32_t)step;
    if (next_index < 0)
    {
        next_index = 0;
    }

    if (next_index >= (int32_t)setting->option.option_count)
    {
        next_index = (int32_t)setting->option.option_count - 1;
    }

    if ((uint16_t)next_index == current_index)
    {
        return PARAMETER_APPLY_OK;
    }

    return menu_setting_backend_commit_option(setting->id,
                                              setting->option.options[next_index].value);
}

/* 执行动作型设置项，例如手动零漂学习。 */
parameter_apply_status_t menu_setting_backend_execute_action(menu_setting_id_t id)
{
    switch (id)
    {
    case MENU_SETTING_ID_ZERO_LEARN:
        return parameter_execute_action(PARAMETER_ACTION_ZERO_LEARN_START);

    default:
        return PARAMETER_APPLY_UNSUPPORTED;
    }
}

/*
 * 根据设置项描述和当前参数构造页面显示视图。
 * 这一步会把参数值转成字符串、单位和提示文本，是设置页渲染前最重要的适配函数。
 */
bool menu_setting_backend_build_view(const menu_setting_desc_t *setting,
                                     menu_setting_view_t *view)
{
    double numeric_value = 0.0;
    uint32_t option_value = 0U;
    const menu_setting_option_t *option_item = NULL;

    if ((setting == NULL) || (view == NULL))
    {
        return false;
    }

    (void)memset(view, 0, sizeof(*view));
    view->unit_text = "";
    view->selected_char_index = -1;
    menu_setting_backend_copy_detail(view, setting->placeholder_text);

    switch (setting->id)
    {
    case MENU_SETTING_ID_PIPE_OD:
    case MENU_SETTING_ID_PIPE_WALL:
        if (!menu_setting_backend_get_numeric_value(setting->id, &numeric_value))
        {
            return false;
        }
        (void)snprintf(view->value_text,
                       sizeof(view->value_text),
                       "%0*.*f",
                       (int)(setting->numeric.integer_digits +
                             ((setting->numeric.fractional_digits > 0U) ? 1U : 0U) +
                             setting->numeric.fractional_digits),
                       (int)setting->numeric.fractional_digits,
                       numeric_value);
        view->unit_text = setting->numeric.unit_text != NULL ? setting->numeric.unit_text : "";
        view->show_value_box = true;
        return true;

    case MENU_SETTING_ID_ALARM_LOW:
    case MENU_SETTING_ID_ALARM_HIGH:
        if (!menu_setting_backend_get_numeric_value(setting->id, &numeric_value))
        {
            return false;
        }
        (void)snprintf(view->value_text,
                       sizeof(view->value_text),
                       "%0*.*f",
                       (int)(setting->numeric.integer_digits +
                             ((setting->numeric.fractional_digits > 0U) ? 1U : 0U) +
                             setting->numeric.fractional_digits),
                       (int)setting->numeric.fractional_digits,
                       numeric_value);
        view->unit_text = rate_unit_to_str(g_parameters.rate_unit_type);
        view->show_value_box = true;
        return true;

    case MENU_SETTING_ID_ZERO_LEARN:
        if (!menu_setting_backend_get_numeric_value(setting->id, &numeric_value))
        {
            return false;
        }
        (void)snprintf(view->value_text, sizeof(view->value_text), "%.3f", numeric_value);
        view->unit_text = "m/s";
        view->show_value_box = true;
        menu_setting_backend_copy_detail(view, "OK RUN");
        return true;

    case MENU_SETTING_ID_PIPE_MATERIAL:
    case MENU_SETTING_ID_OUTPUT_MODE:
    case MENU_SETTING_ID_FLOW_UNIT:
    case MENU_SETTING_ID_REFRESH_RATE:
        if (!menu_setting_backend_get_option_value(setting->id, &option_value))
        {
            return false;
        }

        option_item = menu_setting_backend_find_option(setting, option_value);
        if (option_item != NULL)
        {
            (void)snprintf(view->value_text, sizeof(view->value_text), "%s", option_item->label);
        }
        else
        {
            (void)snprintf(view->value_text, sizeof(view->value_text), "%lu", (unsigned long)option_value);
        }

        view->show_value_box = true;
        if (setting->id == MENU_SETTING_ID_PIPE_MATERIAL)
        {
            menu_setting_backend_copy_detail(view, "UP/DOWN SELECT");
        }
        else if (setting->id == MENU_SETTING_ID_FLOW_UNIT)
        {
            (void)snprintf(view->detail_text,
                           sizeof(view->detail_text),
                           "UP/DOWN SELECT\nTOTAL:%s",
                           volume_unit_to_str(volume_unit_from_rate_unit((RateUnitType)option_value)));
        }
        return true;

    case MENU_SETTING_ID_SYSTEM:
        (void)snprintf(view->detail_text,
                       sizeof(view->detail_text),
                       "NAME:%s\nVER:%s\nADDR:%u\nSAVED:%s",
                       APP_SW_NAME,
                       APP_SW_VERSION,
                       (unsigned int)g_parameters.modbus_addr,
                       g_parameters.is_saved ? "YES" : "NO");
        view->show_value_box = false;
        return true;

    default:
        return false;
    }
}
