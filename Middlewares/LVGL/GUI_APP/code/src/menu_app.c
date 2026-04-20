/*
 * 菜单应用主控文件。
 * 本文件是当前 LVGL 界面的调度中心，负责：
 * 1. 创建各页面组件
 * 2. 处理按键并完成页面切换
 * 3. 从算法与参数全局状态中提取显示数据
 * 4. 维护 GUI 任务的运行时诊断信息
 *
 * 若要理解“按键如何驱动界面”和“测量页为何按某个频率刷新”，这里是主入口。
 */
#include "menu_app.h"

#define LOG_TAG "menu_app"
#define LOG_LVL ELOG_LVL_INFO

#include "bsp_key.h"
#include "data.h"
#include "elog.h"
#include "lv_port_disp_template.h"
#include "menu_data.h"
#include "menu_list_page.h"
#include "menu_measure_page.h"
#include "menu_setting_backend.h"
#include "menu_setting_page.h"
#include "menu_setting_session.h"

#include "FreeRTOS.h"
#include "portable.h"
#include "task.h"

#include <math.h>
#include <string.h>

/* `display_sensitivity` is configured in Hz. */
#define MENU_MEASURE_REFRESH_MIN_HZ 1U
#define MENU_MEASURE_REFRESH_MAX_HZ 25U
#define MENU_RUNTIME_MONITOR_PERIOD_MS 5000U
#define MENU_RUNTIME_WARN_STACK_WORDS 96U
#define MENU_RUNTIME_WARN_RTOS_HEAP_MIN 2048U
#define MENU_RUNTIME_WARN_LVGL_HEAP_MIN 2048U

#define MENU_APP_DIAG_FLAG_STACK_LOW (1UL << 0)
#define MENU_APP_DIAG_FLAG_RTOS_HEAP_LOW (1UL << 1)
#define MENU_APP_DIAG_FLAG_LVGL_HEAP_LOW (1UL << 2)

typedef enum
{
    MENU_SCREEN_MEASURE = 0,
    MENU_SCREEN_LIST,
    MENU_SCREEN_SETTING
} menu_screen_t;

typedef enum
{
    MENU_KEY_NONE = 0,
    MENU_KEY_OK,
    MENU_KEY_UP,
    MENU_KEY_DOWN,
    MENU_KEY_BACK
} menu_key_t;

typedef struct
{
    menu_screen_t current_screen;
    menu_nav_state_t nav_state;
    const menu_setting_desc_t *active_setting;
    menu_setting_session_t setting_session;
    menu_measure_page_t measure_page;
    menu_list_page_t list_page;
    menu_setting_page_t setting_page;
} menu_app_ctx_t;

typedef struct
{
    uint32_t monitor_seq;
    uint32_t current_screen;
    uint32_t last_key;
    uint32_t lvgl_handler_enter_count;
    uint32_t lvgl_handler_exit_count;
    uint32_t menu_tick_enter_count;
    uint32_t menu_tick_exit_count;
    uint32_t measure_render_count;
    uint32_t last_loop_tick;
    uint32_t last_tick_cb_tick;
    uint32_t last_measure_render_tick;
    uint32_t last_monitor_tick;
    uint32_t gui_stack_words;
    uint32_t rtos_free_heap_bytes;
    uint32_t rtos_min_ever_free_heap_bytes;
    uint32_t lvgl_free_bytes;
    uint32_t lvgl_biggest_free_bytes;
    uint32_t lvgl_frag_pct;
    uint32_t warning_flags;
} menu_app_diag_snapshot_t;

static menu_app_ctx_t s_menu_app;
static uint32_t s_last_measure_render_tick = 0U;
static uint32_t s_last_runtime_monitor_tick = 0U;

/* Visible in Ozone to check whether the GUI task is still making progress. */
volatile menu_app_diag_snapshot_t g_menu_app_diag_snapshot;

/* 将底层按键值转换为菜单系统内部统一使用的按键枚举。 */
static menu_key_t menu_app_map_key(uint8_t raw_key)
{
    switch (raw_key)
    {
    case KEY1_PRESS:
    case KEY1_LONG_PRESS:
        return MENU_KEY_OK;

    case KEY2_PRESS:
    case KEY2_LONG_PRESS:
        return MENU_KEY_UP;

    case KEY3_PRESS:
    case KEY3_LONG_PRESS:
        return MENU_KEY_DOWN;

    case KEY4_PRESS:
    case KEY4_LONG_PRESS:
        return MENU_KEY_BACK;

    default:
        return MENU_KEY_NONE;
    }
}

/* 计算圆弧表盘的满量程，使用当前流量单位下的报警上限作为视觉满量程。 */
static double menu_app_get_arc_full_scale(void)
{
    double full_scale = fabs(g_parameters.alarm_upper_rate_range);

    if (full_scale < MENU_MIN_ARC_FULL_SCALE)
    {
        full_scale = 1.0;
    }

    return full_scale;
}

/* 根据 `display_sensitivity` 计算测量页刷新周期，并做上下限保护。 */
static uint32_t menu_app_get_measure_refresh_period_ms(void)
{
    uint32_t refresh_hz = g_parameters.display_sensitivity;
    uint32_t period_ms;

    if (refresh_hz < MENU_MEASURE_REFRESH_MIN_HZ)
    {
        refresh_hz = MENU_MEASURE_REFRESH_MIN_HZ;
    }
    else if (refresh_hz > MENU_MEASURE_REFRESH_MAX_HZ)
    {
        refresh_hz = MENU_MEASURE_REFRESH_MAX_HZ;
    }

    period_ms = 1000U / refresh_hz;
    if (period_ms < MENU_KEY_POLL_PERIOD_MS)
    {
        period_ms = MENU_KEY_POLL_PERIOD_MS;
    }

    return period_ms;
}

/*
 * 采集 GUI 任务运行时状态。
 * 主要用于观察：
 * 1. GUI 任务栈余量
 * 2. RTOS heap 余量
 * 3. LVGL 内存碎片与最大空闲块
 */
static void menu_app_monitor_runtime(void)
{
    lv_mem_monitor_t mem_mon;
    UBaseType_t stack_words;
    uint32_t rtos_free;
    uint32_t warning_flags = 0U;

    if ((s_last_runtime_monitor_tick != 0U) &&
        (lv_tick_elaps(s_last_runtime_monitor_tick) < MENU_RUNTIME_MONITOR_PERIOD_MS))
    {
        return;
    }

    s_last_runtime_monitor_tick = lv_tick_get();
    lv_mem_monitor(&mem_mon);
    stack_words = uxTaskGetStackHighWaterMark(NULL);
    rtos_free = (uint32_t)xPortGetFreeHeapSize();

    if (stack_words < MENU_RUNTIME_WARN_STACK_WORDS)
    {
        warning_flags |= MENU_APP_DIAG_FLAG_STACK_LOW;
    }

    if (rtos_free < MENU_RUNTIME_WARN_RTOS_HEAP_MIN)
    {
        warning_flags |= MENU_APP_DIAG_FLAG_RTOS_HEAP_LOW;
    }

    if (mem_mon.free_biggest_size < MENU_RUNTIME_WARN_LVGL_HEAP_MIN)
    {
        warning_flags |= MENU_APP_DIAG_FLAG_LVGL_HEAP_LOW;
    }

    g_menu_app_diag_snapshot.monitor_seq++;
    g_menu_app_diag_snapshot.current_screen = (uint32_t)s_menu_app.current_screen;
    g_menu_app_diag_snapshot.last_monitor_tick = s_last_runtime_monitor_tick;
    g_menu_app_diag_snapshot.gui_stack_words = (uint32_t)stack_words;
    g_menu_app_diag_snapshot.rtos_free_heap_bytes = rtos_free;
    g_menu_app_diag_snapshot.rtos_min_ever_free_heap_bytes = (uint32_t)xPortGetMinimumEverFreeHeapSize();
    g_menu_app_diag_snapshot.lvgl_free_bytes = (uint32_t)mem_mon.free_size;
    g_menu_app_diag_snapshot.lvgl_biggest_free_bytes = (uint32_t)mem_mon.free_biggest_size;
    g_menu_app_diag_snapshot.lvgl_frag_pct = (uint32_t)mem_mon.frag_pct;
    g_menu_app_diag_snapshot.warning_flags = warning_flags;

    if (warning_flags != 0U)
    {
        log_w("GUI margin low: stack=%lu words, rtos_free=%lu, lvgl_free=%lu, lvgl_biggest=%lu, lvgl_frag=%u%%",
              (unsigned long)stack_words,
              (unsigned long)rtos_free,
              (unsigned long)mem_mon.free_size,
              (unsigned long)mem_mon.free_biggest_size,
              (unsigned int)mem_mon.frag_pct);
    }
}

/* 统一隐藏所有页面，随后再只显示目标页面。 */
static void menu_app_hide_all(menu_app_ctx_t *app)
{
    menu_measure_page_set_visible(&app->measure_page, false);
    menu_list_page_set_visible(&app->list_page, false);
    menu_setting_page_set_visible(&app->setting_page, false);
}

/*
 * 渲染测量页。
 * 它会从全局算法输出中读取 SQ、瞬时流量和累计流量，再换算成测量页需要的圆弧值和文本。
 */
static void menu_app_render_measure(menu_app_ctx_t *app)
{
    double sq_value = g_algo_out.sq_value;
    double instant_flow_value = g_algo_out.flow_rate_instant;
    double total_flow_value = g_algo_out.flow_rate_total;
    double full_scale = menu_app_get_arc_full_scale();
    double ratio;
    int16_t arc_value;
    const char *flow_unit_text = rate_unit_to_str(g_algo_out.flow_rate_unit);
    const char *total_unit_text = volume_unit_to_str(g_algo_out.flow_total_unit);

    if (sq_value < 0.0)
    {
        sq_value = 0.0;
    }
    else if (sq_value > 100.0)
    {
        sq_value = 100.0;
    }

    ratio = fabs(instant_flow_value) / full_scale;
    if (ratio > 1.0)
    {
        ratio = 1.0;
    }

    arc_value = (int16_t)(ratio * (double)MENU_ARC_VALUE_MAX + 0.5);

    menu_measure_page_render(&app->measure_page,
                             sq_value,
                             instant_flow_value,
                             flow_unit_text,
                             full_scale,
                             total_flow_value,
                             total_unit_text,
                             arc_value);
    s_last_measure_render_tick = lv_tick_get();
    g_menu_app_diag_snapshot.measure_render_count++;
    g_menu_app_diag_snapshot.last_measure_render_tick = s_last_measure_render_tick;
    g_menu_app_diag_snapshot.current_screen = (uint32_t)app->current_screen;
}

/* 刷新菜单列表页。 */
static void menu_app_render_list(menu_app_ctx_t *app)
{
    menu_list_page_render(&app->list_page, &app->nav_state);
}

/* 刷新设置页。 */
static void menu_app_render_setting(menu_app_ctx_t *app)
{
    menu_setting_view_t view;

    if (app == NULL)
    {
        return;
    }

    if ((app->active_setting != NULL) &&
        menu_setting_session_build_view(&app->setting_session, &view))
    {
        menu_setting_page_render(&app->setting_page, app->active_setting, &view);
    }
    else
    {
        menu_setting_page_render(&app->setting_page, app->active_setting, NULL);
    }
}

/* 切换到测量页。 */
static void menu_app_show_measure(menu_app_ctx_t *app)
{
    app->current_screen = MENU_SCREEN_MEASURE;
    menu_app_hide_all(app);
    menu_measure_page_set_visible(&app->measure_page, true);
}

/* 切换到列表页。 */
static void menu_app_show_list(menu_app_ctx_t *app)
{
    app->current_screen = MENU_SCREEN_LIST;
    menu_app_hide_all(app);
    menu_list_page_set_visible(&app->list_page, true);
}

/* 切换到设置页。 */
static void menu_app_show_setting(menu_app_ctx_t *app)
{
    app->current_screen = MENU_SCREEN_SETTING;
    menu_app_hide_all(app);
    menu_setting_page_set_visible(&app->setting_page, true);
}

/* 打开菜单根页。 */
static void menu_app_open_root(menu_app_ctx_t *app)
{
    menu_nav_reset_root(&app->nav_state, menu_data_get_root_page());
    app->active_setting = NULL;
    menu_app_show_list(app);
    menu_app_render_list(app);
}

/* 从菜单或设置页回到测量页。 */
static void menu_app_back_to_measure(menu_app_ctx_t *app)
{
    menu_nav_reset_root(&app->nav_state, menu_data_get_root_page());
    app->active_setting = NULL;
    menu_app_show_measure(app);
    menu_app_render_measure(app);
}

/* 打开某一项设置并初始化本次编辑会话。 */
static void menu_app_open_setting(menu_app_ctx_t *app, const menu_setting_desc_t *setting)
{
    app->active_setting = setting;
    (void)menu_setting_session_begin(&app->setting_session, setting);
    menu_app_show_setting(app);
    menu_app_render_setting(app);
}

/* 返回上一级菜单。 */
static void menu_app_back_to_parent(menu_app_ctx_t *app)
{
    if (menu_nav_pop(&app->nav_state))
    {
        menu_nav_reset_current_selection(&app->nav_state);
    }

    app->active_setting = NULL;
    menu_app_show_list(app);
    menu_app_render_list(app);
}

/* 从设置页退出并回到列表页。 */
static void menu_app_back_from_setting(menu_app_ctx_t *app)
{
    menu_nav_reset_current_selection(&app->nav_state);
    menu_setting_session_reset(&app->setting_session);
    app->active_setting = NULL;
    menu_app_show_list(app);
    menu_app_render_list(app);
}

/* 处理列表页中的“确认/进入”动作。 */
static void menu_app_handle_ok(menu_app_ctx_t *app)
{
    menu_entry_type_t entry_type = MENU_ENTRY_MEASURE;
    const menu_page_t *submenu = NULL;
    const menu_setting_desc_t *setting = NULL;

    if (!menu_nav_enter(&app->nav_state, &entry_type, &submenu, &setting))
    {
        return;
    }

    switch (entry_type)
    {
    case MENU_ENTRY_MEASURE:
        menu_app_back_to_measure(app);
        break;

    case MENU_ENTRY_SUBMENU:
        menu_app_show_list(app);
        menu_app_render_list(app);
        break;

    case MENU_ENTRY_SETTING:
        menu_app_open_setting(app, setting);
        break;

    default:
        break;
    }
}

/*
 * 菜单按键分发函数。
 * 它会根据当前页面把同一个按键解释成不同动作，例如：
 * - 在列表页中，上下键表示移动选中项
 * - 在设置页中，上下键表示修改当前参数
 * - 在测量页中，返回键表示进入主菜单
 */
static void menu_app_handle_key(menu_app_ctx_t *app, menu_key_t key)
{
    g_menu_app_diag_snapshot.last_key = (uint32_t)key;
    g_menu_app_diag_snapshot.current_screen = (uint32_t)app->current_screen;

    switch (app->current_screen)
    {
    case MENU_SCREEN_MEASURE:
        if (key == MENU_KEY_BACK)
        {
            menu_app_open_root(app);
        }
        break;

    case MENU_SCREEN_LIST:
        switch (key)
        {
        case MENU_KEY_UP:
            if (menu_nav_move_up(&app->nav_state))
            {
                menu_app_render_list(app);
            }
            break;

        case MENU_KEY_DOWN:
            if (menu_nav_move_down(&app->nav_state))
            {
                menu_app_render_list(app);
            }
            break;

        case MENU_KEY_OK:
            menu_app_handle_ok(app);
            break;

        case MENU_KEY_BACK:
            if (menu_nav_can_pop(&app->nav_state))
            {
                menu_app_back_to_parent(app);
            }
            else
            {
                menu_app_back_to_measure(app);
            }
            break;

        case MENU_KEY_NONE:
        default:
            break;
        }
        break;

    case MENU_SCREEN_SETTING:
        if (key == MENU_KEY_BACK)
        {
            menu_app_back_from_setting(app);
        }
        else if ((key == MENU_KEY_UP) &&
                 menu_setting_session_is_active(&app->setting_session))
        {
            if (menu_setting_session_step_up(&app->setting_session))
            {
                menu_app_render_setting(app);
            }
        }
        else if ((key == MENU_KEY_DOWN) &&
                 menu_setting_session_is_active(&app->setting_session))
        {
            if (menu_setting_session_step_down(&app->setting_session))
            {
                menu_app_render_setting(app);
            }
        }
        else if ((key == MENU_KEY_OK) &&
                 menu_setting_session_is_active(&app->setting_session))
        {
            menu_setting_session_event_t setting_event =
                menu_setting_session_confirm(&app->setting_session);

            if (setting_event == MENU_SETTING_SESSION_EVENT_CLOSE)
            {
                menu_app_back_from_setting(app);
            }
            else if ((setting_event == MENU_SETTING_SESSION_EVENT_UPDATED) ||
                     (setting_event == MENU_SETTING_SESSION_EVENT_ERROR))
            {
                menu_app_render_setting(app);
            }
        }
        break;

    default:
        break;
    }
}

/* 创建菜单应用上下文和各页面控件，系统启动时调用一次。 */
static void menu_app_create(menu_app_ctx_t *app)
{
    lv_obj_t *scr = lv_scr_act();

    if (app == NULL)
    {
        return;
    }

    (void)memset(app, 0, sizeof(*app));

    lv_obj_set_style_bg_color(scr, lv_color_hex(MENU_COLOR_BLACK), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    menu_measure_page_create(&app->measure_page, scr);
    menu_list_page_create(&app->list_page, scr);
    menu_setting_page_create(&app->setting_page, scr);

    menu_nav_init(&app->nav_state, menu_data_get_root_page());
    menu_app_hide_all(app);
    menu_app_show_measure(app);
    menu_app_render_measure(app);
}

/*
 * LVGL 定时回调。
 * 它相当于 GUI 的“业务轮询心跳”，主要负责：
 * 1. 读取按键
 * 2. 分发页面逻辑
 * 3. 控制测量页刷新频率
 * 4. 采集 GUI 诊断信息
 */
static void menu_app_tick_cb(lv_timer_t *timer)
{
    menu_key_t key;

    LV_UNUSED(timer);

    g_menu_app_diag_snapshot.menu_tick_enter_count++;
    g_menu_app_diag_snapshot.last_tick_cb_tick = (uint32_t)xTaskGetTickCount();
    g_menu_app_diag_snapshot.current_screen = (uint32_t)s_menu_app.current_screen;

    key = menu_app_map_key(key_scan(0U));
    if (key != MENU_KEY_NONE)
    {
        menu_app_handle_key(&s_menu_app, key);
    }

    if (s_menu_app.current_screen == MENU_SCREEN_MEASURE)
    {
        if ((s_last_measure_render_tick == 0U) ||
            (lv_tick_elaps(s_last_measure_render_tick) >= menu_app_get_measure_refresh_period_ms()))
        {
            menu_app_render_measure(&s_menu_app);
        }
    }

    menu_app_monitor_runtime();
    g_menu_app_diag_snapshot.menu_tick_exit_count++;
}

/*
 * GUI 主任务。
 * 本任务完成 LVGL 初始化后进入无限循环，持续调用 `lv_timer_handler()`，
 * 也就是整个图形界面的核心调度入口。
 */
void menu_app_task(void *p)
{
    (void)p;

    lv_init();
    lv_port_disp_init();
    menu_app_create(&s_menu_app);
    lv_timer_create(menu_app_tick_cb, MENU_KEY_POLL_PERIOD_MS, NULL);

    while (1)
    {
        g_menu_app_diag_snapshot.lvgl_handler_enter_count++;
        lv_timer_handler();
        g_menu_app_diag_snapshot.lvgl_handler_exit_count++;
        g_menu_app_diag_snapshot.last_loop_tick = (uint32_t)xTaskGetTickCount();
        g_menu_app_diag_snapshot.current_screen = (uint32_t)s_menu_app.current_screen;
        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}
