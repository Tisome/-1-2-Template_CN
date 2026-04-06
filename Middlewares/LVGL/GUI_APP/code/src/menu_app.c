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
#define MENU_MEASURE_REFRESH_MIN_HZ      1U
#define MENU_MEASURE_REFRESH_MAX_HZ      25U
#define MENU_RUNTIME_MONITOR_PERIOD_MS   5000U
#define MENU_RUNTIME_WARN_STACK_WORDS    96U
#define MENU_RUNTIME_WARN_RTOS_HEAP_MIN  2048U
#define MENU_RUNTIME_WARN_LVGL_HEAP_MIN  2048U

#define MENU_APP_DIAG_FLAG_STACK_LOW     (1UL << 0)
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

static double menu_app_rate_to_m3ps(double rate_value, RateUnitType unit)
{
    switch (unit)
    {
    case RATE_UNIT_M3_P_H:
        return rate_value / 3600.0;

    case RATE_UNIT_M3_P_MIN:
        return rate_value / 60.0;

    case RATE_UNIT_L_P_H:
        return rate_value / (1000.0 * 3600.0);

    case RATE_UNIT_L_P_MIN:
        return rate_value / (1000.0 * 60.0);

    case RATE_UNIT_L_P_S:
        return rate_value / 1000.0;

    case RATE_UNIT_M3_P_S:
    default:
        return rate_value;
    }
}

static double menu_app_get_instant_flow_m3ph(void)
{
    double flow_m3ps = menu_app_rate_to_m3ps(g_algo_out.flow_rate_instant, g_algo_out.flow_rate_unit);

    return convert_rate_from_m3ps(flow_m3ps, RATE_UNIT_M3_P_H);
}

static double menu_app_get_arc_full_scale_m3ph(void)
{
    double full_scale_m3ps = menu_app_rate_to_m3ps(g_parameters.alarm_upper_rate_range, g_parameters.rate_unit_type);
    double full_scale_m3ph = fabs(convert_rate_from_m3ps(full_scale_m3ps, RATE_UNIT_M3_P_H));

    if (full_scale_m3ph < MENU_MIN_ARC_FULL_SCALE)
    {
        full_scale_m3ph = 1.0;
    }

    return full_scale_m3ph;
}

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

static void menu_app_hide_all(menu_app_ctx_t *app)
{
    menu_measure_page_set_visible(&app->measure_page, false);
    menu_list_page_set_visible(&app->list_page, false);
    menu_setting_page_set_visible(&app->setting_page, false);
}

static void menu_app_render_measure(menu_app_ctx_t *app)
{
    double sq_value = g_algo_out.sq_value;
    double instant_flow_m3ph = menu_app_get_instant_flow_m3ph();
    double total_flow_m3 = g_algo_state.q_total_m3;
    double full_scale_m3ph = menu_app_get_arc_full_scale_m3ph();
    double ratio;
    int16_t arc_value;

    if (sq_value < 0.0)
    {
        sq_value = 0.0;
    }
    else if (sq_value > 100.0)
    {
        sq_value = 100.0;
    }

    ratio = fabs(instant_flow_m3ph) / full_scale_m3ph;
    if (ratio > 1.0)
    {
        ratio = 1.0;
    }

    arc_value = (int16_t)(ratio * (double)MENU_ARC_VALUE_MAX + 0.5);

    menu_measure_page_render(&app->measure_page,
                             sq_value,
                             instant_flow_m3ph,
                             total_flow_m3,
                             arc_value);
    s_last_measure_render_tick = lv_tick_get();
    g_menu_app_diag_snapshot.measure_render_count++;
    g_menu_app_diag_snapshot.last_measure_render_tick = s_last_measure_render_tick;
    g_menu_app_diag_snapshot.current_screen = (uint32_t)app->current_screen;
}

static void menu_app_render_list(menu_app_ctx_t *app)
{
    menu_list_page_render(&app->list_page, &app->nav_state);
}

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

static void menu_app_show_measure(menu_app_ctx_t *app)
{
    app->current_screen = MENU_SCREEN_MEASURE;
    menu_app_hide_all(app);
    menu_measure_page_set_visible(&app->measure_page, true);
}

static void menu_app_show_list(menu_app_ctx_t *app)
{
    app->current_screen = MENU_SCREEN_LIST;
    menu_app_hide_all(app);
    menu_list_page_set_visible(&app->list_page, true);
}

static void menu_app_show_setting(menu_app_ctx_t *app)
{
    app->current_screen = MENU_SCREEN_SETTING;
    menu_app_hide_all(app);
    menu_setting_page_set_visible(&app->setting_page, true);
}

static void menu_app_open_root(menu_app_ctx_t *app)
{
    menu_nav_reset_root(&app->nav_state, menu_data_get_root_page());
    app->active_setting = NULL;
    menu_app_show_list(app);
    menu_app_render_list(app);
}

static void menu_app_back_to_measure(menu_app_ctx_t *app)
{
    menu_nav_reset_root(&app->nav_state, menu_data_get_root_page());
    app->active_setting = NULL;
    menu_app_show_measure(app);
    menu_app_render_measure(app);
}

static void menu_app_open_setting(menu_app_ctx_t *app, const menu_setting_desc_t *setting)
{
    app->active_setting = setting;
    (void)menu_setting_session_begin(&app->setting_session, setting);
    menu_app_show_setting(app);
    menu_app_render_setting(app);
}

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

static void menu_app_back_from_setting(menu_app_ctx_t *app)
{
    menu_nav_reset_current_selection(&app->nav_state);
    menu_setting_session_reset(&app->setting_session);
    app->active_setting = NULL;
    menu_app_show_list(app);
    menu_app_render_list(app);
}

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
