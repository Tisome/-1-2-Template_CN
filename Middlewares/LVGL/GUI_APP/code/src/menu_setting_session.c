/*
 * 设置会话状态机文件。
 * 本文件维护“某个设置项正在被编辑”时的临时状态，包括：
 * 1. 当前值字符串
 * 2. 当前选中的数字位
 * 3. 最近一次提交结果
 * 4. 数值型或选项型设置项的待提交值
 *
 * GUI 按键并不直接改全局参数，而是先驱动这里的会话状态，
 * 最终再由后端统一提交。
 */
#include "menu_setting_session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 计算数值型设置项格式化后需要的总字符宽度。 */
static uint8_t menu_setting_session_format_width(const menu_setting_desc_t *setting)
{
    uint8_t width = 0U;

    if (setting == NULL)
    {
        return 0U;
    }

    width = setting->numeric.integer_digits;
    if (setting->numeric.fractional_digits > 0U)
    {
        width = (uint8_t)(width + 1U + setting->numeric.fractional_digits);
    }

    return width;
}

/* 按设置项的小数位要求，将当前数值格式化为可编辑字符串。 */
static void menu_setting_session_format_numeric(menu_setting_session_t *session)
{
    uint8_t text_width;

    if ((session == NULL) || (session->setting == NULL))
    {
        return;
    }

    text_width = menu_setting_session_format_width(session->setting);
    (void)snprintf(session->value_text,
                   sizeof(session->value_text),
                   "%0*.*f",
                   (int)text_width,
                   (int)session->setting->numeric.fractional_digits,
                   session->numeric_value);
}

/* 更新字符串中每一位数字的位置，便于按键逐位编辑。 */
static void menu_setting_session_update_digit_positions(menu_setting_session_t *session)
{
    uint8_t char_index;
    uint8_t digit_count = 0U;

    if (session == NULL)
    {
        return;
    }

    session->digit_count = 0U;
    session->selected_digit_index = -1;

    for (char_index = 0U; session->value_text[char_index] != '\0'; char_index++)
    {
        if ((session->value_text[char_index] >= '0') &&
            (session->value_text[char_index] <= '9') &&
            (digit_count < (uint8_t)sizeof(session->digit_positions)))
        {
            session->digit_positions[digit_count] = char_index;
            digit_count++;
        }
    }

    session->digit_count = digit_count;
    if (digit_count > 0U)
    {
        session->selected_digit_index = (int8_t)(digit_count - 1U);
    }
}

/* 将参数应用结果转换成界面可直接显示的状态文本。 */
static const char *menu_setting_session_status_text(parameter_apply_status_t status)
{
    switch (status)
    {
    case PARAMETER_APPLY_OK:
        return "SAVE OK";

    case PARAMETER_APPLY_INVALID:
        return "INVALID PARAM";

    case PARAMETER_APPLY_SAVE_FAILED:
        return "SAVE FAIL";

    case PARAMETER_APPLY_BUSY:
        return "STORAGE BUSY";

    case PARAMETER_APPLY_UNSUPPORTED:
    default:
        return "UNSUPPORTED";
    }
}

/* 根据当前会话状态刷新底部提示语或保存结果提示。 */
static void menu_setting_session_update_detail(menu_setting_session_t *session)
{
    const char *status_text = NULL;

    if (session == NULL)
    {
        return;
    }

    session->detail_text[0] = '\0';

    if (!session->active || (session->setting == NULL))
    {
        return;
    }

    if ((session->last_status != PARAMETER_APPLY_OK) &&
        (session->last_status != PARAMETER_APPLY_UNSUPPORTED))
    {
        status_text = menu_setting_session_status_text(session->last_status);
    }

    switch (session->setting->kind)
    {
    case MENU_SETTING_KIND_NUMERIC:
        if (status_text != NULL)
        {
            (void)snprintf(session->detail_text, sizeof(session->detail_text), "%s", status_text);
        }
        else
        {
            (void)snprintf(session->detail_text,
                           sizeof(session->detail_text),
                           "UP/DOWN EDIT\nOK NEXT/SAVE");
        }
        break;

    case MENU_SETTING_KIND_OPTION:
        if (status_text != NULL)
        {
            (void)snprintf(session->detail_text, sizeof(session->detail_text), "%s", status_text);
        }
        else
        {
            (void)snprintf(session->detail_text,
                           sizeof(session->detail_text),
                           "UP/DOWN SELECT\nOK SAVE");
        }
        break;

    case MENU_SETTING_KIND_ACTION:
        if (status_text != NULL)
        {
            (void)snprintf(session->detail_text, sizeof(session->detail_text), "%s", status_text);
        }
        else
        {
            (void)snprintf(session->detail_text, sizeof(session->detail_text), "OK RUN");
        }
        break;

    case MENU_SETTING_KIND_INFO:
    default:
        break;
    }
}

/* 将当前数值字符串解析回 double，供范围校验和提交使用。 */
static bool menu_setting_session_parse_numeric(menu_setting_session_t *session, double *value)
{
    double parsed_value = 0.0;
    char *end_ptr = NULL;

    if ((session == NULL) || (value == NULL))
    {
        return false;
    }

    parsed_value = strtod(session->value_text, &end_ptr);
    if ((end_ptr == NULL) || (*end_ptr != '\0'))
    {
        return false;
    }

    *value = parsed_value;
    return true;
}

/*
 * 修改当前选中的数字位。
 * 该函数会先修改字符串中的目标字符，再尝试重新解析成数值；
 * 如果解析失败或越界，会回滚修改，保证编辑过程始终处于有效状态。
 */
static bool menu_setting_session_change_numeric_digit(menu_setting_session_t *session, int8_t delta)
{
    char original_char;
    uint8_t char_index;
    double parsed_value = 0.0;

    if ((session == NULL) ||
        (session->setting == NULL) ||
        (session->selected_digit_index < 0) ||
        ((uint8_t)session->selected_digit_index >= session->digit_count))
    {
        return false;
    }

    char_index = session->digit_positions[(uint8_t)session->selected_digit_index];
    original_char = session->value_text[char_index];

    if ((delta > 0) && (original_char >= '9'))
    {
        return false;
    }

    if ((delta < 0) && (original_char <= '0'))
    {
        return false;
    }

    session->value_text[char_index] = (char)(original_char + delta);
    if (!menu_setting_session_parse_numeric(session, &parsed_value))
    {
        session->value_text[char_index] = original_char;
        return false;
    }

    if ((parsed_value < session->setting->numeric.min_value) ||
        (parsed_value > session->setting->numeric.max_value))
    {
        session->value_text[char_index] = original_char;
        return false;
    }

    session->numeric_value = parsed_value;
    session->dirty = true;
    session->last_status = PARAMETER_APPLY_OK;
    menu_setting_session_update_detail(session);
    return true;
}

/* 在选项型设置项中按步进切换候选值。 */
static bool menu_setting_session_change_option(menu_setting_session_t *session, int8_t step)
{
    uint16_t option_index;
    uint16_t current_index = 0U;
    int32_t next_index;

    if ((session == NULL) ||
        (session->setting == NULL) ||
        (session->setting->option.options == NULL) ||
        (session->setting->option.option_count == 0U))
    {
        return false;
    }

    for (option_index = 0U; option_index < session->setting->option.option_count; option_index++)
    {
        if (session->setting->option.options[option_index].value == session->option_value)
        {
            current_index = option_index;
            break;
        }
    }

    next_index = (int32_t)current_index + (int32_t)step;
    if (next_index < 0)
    {
        next_index = 0;
    }

    if (next_index >= (int32_t)session->setting->option.option_count)
    {
        next_index = (int32_t)session->setting->option.option_count - 1;
    }

    if ((uint16_t)next_index == current_index)
    {
        return false;
    }

    session->option_value = session->setting->option.options[next_index].value;
    session->dirty = true;
    session->last_status = PARAMETER_APPLY_OK;
    menu_setting_session_update_detail(session);
    return true;
}

/* 清空整个设置会话状态。 */
void menu_setting_session_reset(menu_setting_session_t *session)
{
    if (session == NULL)
    {
        return;
    }

    (void)memset(session, 0, sizeof(*session));
    session->selected_digit_index = -1;
    session->last_status = PARAMETER_APPLY_OK;
}

/* 为指定设置项开启新的编辑会话，并装载当前参数值。 */
bool menu_setting_session_begin(menu_setting_session_t *session, const menu_setting_desc_t *setting)
{
    uint32_t option_value = 0U;
    double numeric_value = 0.0;

    if ((session == NULL) || (setting == NULL))
    {
        return false;
    }

    menu_setting_session_reset(session);
    session->setting = setting;
    session->active = true;
    session->last_status = PARAMETER_APPLY_OK;

    switch (setting->kind)
    {
    case MENU_SETTING_KIND_NUMERIC:
        if (!menu_setting_backend_get_numeric_value(setting->id, &numeric_value))
        {
            return false;
        }
        session->numeric_value = numeric_value;
        menu_setting_session_format_numeric(session);
        menu_setting_session_update_digit_positions(session);
        break;

    case MENU_SETTING_KIND_OPTION:
        if (!menu_setting_backend_get_option_value(setting->id, &option_value))
        {
            return false;
        }
        session->option_value = option_value;
        break;

    case MENU_SETTING_KIND_ACTION:
    case MENU_SETTING_KIND_INFO:
    default:
        break;
    }

    menu_setting_session_update_detail(session);
    return true;
}

/* 判断当前是否存在有效的设置会话。 */
bool menu_setting_session_is_active(const menu_setting_session_t *session)
{
    return (session != NULL) && session->active && (session->setting != NULL);
}

/* 响应“上”键：数值型设置加当前位，选项型设置切到上一个候选值。 */
bool menu_setting_session_step_up(menu_setting_session_t *session)
{
    if (!menu_setting_session_is_active(session))
    {
        return false;
    }

    switch (session->setting->kind)
    {
    case MENU_SETTING_KIND_NUMERIC:
        return menu_setting_session_change_numeric_digit(session, 1);

    case MENU_SETTING_KIND_OPTION:
        return menu_setting_session_change_option(session, -1);

    default:
        return false;
    }
}

/* 响应“下”键：数值型设置减当前位，选项型设置切到下一个候选值。 */
bool menu_setting_session_step_down(menu_setting_session_t *session)
{
    if (!menu_setting_session_is_active(session))
    {
        return false;
    }

    switch (session->setting->kind)
    {
    case MENU_SETTING_KIND_NUMERIC:
        return menu_setting_session_change_numeric_digit(session, -1);

    case MENU_SETTING_KIND_OPTION:
        return menu_setting_session_change_option(session, 1);

    default:
        return false;
    }
}

/*
 * 响应“确认”键。
 * 对数值型设置，前几次确认用于把光标逐位左移，直到最后一位才真正提交；
 * 对选项型和动作型设置，则会直接调用后端提交或执行动作。
 */
menu_setting_session_event_t menu_setting_session_confirm(menu_setting_session_t *session)
{
    parameter_apply_status_t apply_status = PARAMETER_APPLY_UNSUPPORTED;

    if (!menu_setting_session_is_active(session))
    {
        return MENU_SETTING_SESSION_EVENT_NONE;
    }

    switch (session->setting->kind)
    {
    case MENU_SETTING_KIND_NUMERIC:
        if (session->selected_digit_index > 0)
        {
            session->selected_digit_index--;
            menu_setting_session_update_detail(session);
            return MENU_SETTING_SESSION_EVENT_UPDATED;
        }

        apply_status = menu_setting_backend_commit_numeric(session->setting->id,
                                                           session->numeric_value);
        session->last_status = apply_status;
        if ((apply_status != PARAMETER_APPLY_OK) && (session->digit_count > 0U))
        {
            session->selected_digit_index = (int8_t)(session->digit_count - 1U);
        }
        menu_setting_session_update_detail(session);
        return (apply_status == PARAMETER_APPLY_OK) ? MENU_SETTING_SESSION_EVENT_CLOSE
                                                    : MENU_SETTING_SESSION_EVENT_ERROR;

    case MENU_SETTING_KIND_OPTION:
        apply_status = menu_setting_backend_commit_option(session->setting->id,
                                                          session->option_value);
        session->last_status = apply_status;
        menu_setting_session_update_detail(session);
        return (apply_status == PARAMETER_APPLY_OK) ? MENU_SETTING_SESSION_EVENT_CLOSE
                                                    : MENU_SETTING_SESSION_EVENT_ERROR;

    case MENU_SETTING_KIND_ACTION:
        apply_status = menu_setting_backend_execute_action(session->setting->id);
        session->last_status = apply_status;
        menu_setting_session_update_detail(session);
        return (apply_status == PARAMETER_APPLY_OK) ? MENU_SETTING_SESSION_EVENT_UPDATED
                                                    : MENU_SETTING_SESSION_EVENT_ERROR;

    case MENU_SETTING_KIND_INFO:
    default:
        return MENU_SETTING_SESSION_EVENT_NONE;
    }
}

/* 把当前会话状态整理成页面渲染需要的只读视图。 */
bool menu_setting_session_build_view(const menu_setting_session_t *session, menu_setting_view_t *view)
{
    uint16_t option_index;

    if ((session == NULL) || (view == NULL) || !menu_setting_session_is_active(session))
    {
        return false;
    }

    if (!menu_setting_backend_build_view(session->setting, view))
    {
        return false;
    }

    view->selected_char_index = -1;

    switch (session->setting->kind)
    {
    case MENU_SETTING_KIND_NUMERIC:
        (void)snprintf(view->value_text, sizeof(view->value_text), "%s", session->value_text);
        (void)snprintf(view->detail_text, sizeof(view->detail_text), "%s", session->detail_text);
        if ((session->selected_digit_index >= 0) &&
            ((uint8_t)session->selected_digit_index < session->digit_count))
        {
            view->selected_char_index =
                (int8_t)session->digit_positions[(uint8_t)session->selected_digit_index];
        }
        return true;

    case MENU_SETTING_KIND_OPTION:
        for (option_index = 0U; option_index < session->setting->option.option_count; option_index++)
        {
            if (session->setting->option.options[option_index].value == session->option_value)
            {
                (void)snprintf(view->value_text,
                               sizeof(view->value_text),
                               "%s",
                               session->setting->option.options[option_index].label);
                break;
            }
        }

        (void)snprintf(view->detail_text, sizeof(view->detail_text), "%s", session->detail_text);
        return true;

    case MENU_SETTING_KIND_ACTION:
        (void)snprintf(view->detail_text, sizeof(view->detail_text), "%s", session->detail_text);
        return true;

    case MENU_SETTING_KIND_INFO:
    default:
        return true;
    }
}
