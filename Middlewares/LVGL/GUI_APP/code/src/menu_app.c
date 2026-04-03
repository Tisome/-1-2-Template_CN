#include "menu_app.h"

#include "bsp_key.h"
#include "data.h"
#include "lv_port_disp_template.h"
#include "menu_data.h"
#include "menu_list_page.h"
#include "menu_measure_page.h"
#include "menu_setting_page.h"

#include "FreeRTOS.h"
#include "task.h"

#include <math.h>
#include <string.h>

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
    menu_measure_page_t measure_page;
    menu_list_page_t list_page;
    menu_setting_page_t setting_page;
} menu_app_ctx_t;

static menu_app_ctx_t s_menu_app;

static menu_key_t menu_app_map_key(uint8_t raw_key)
{
    switch (raw_key)
    {
    case KEY1_PRESS:
    case KEY1_LONG_PRESS:
        return MENU_KEY_BACK;

    case KEY2_PRESS:
    case KEY2_LONG_PRESS:
        return MENU_KEY_DOWN;

    case KEY3_PRESS:
    case KEY3_LONG_PRESS:
        return MENU_KEY_UP;

    case KEY4_PRESS:
    case KEY4_LONG_PRESS:
        return MENU_KEY_OK;

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
}

static void menu_app_render_list(menu_app_ctx_t *app)
{
    menu_list_page_render(&app->list_page, &app->nav_state);
}

static void menu_app_render_setting(menu_app_ctx_t *app)
{
    menu_setting_page_render(&app->setting_page, app->active_setting);
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

    key = menu_app_map_key(key_scan(0U));
    if (key != MENU_KEY_NONE)
    {
        menu_app_handle_key(&s_menu_app, key);
    }

    if (s_menu_app.current_screen == MENU_SCREEN_MEASURE)
    {
        menu_app_render_measure(&s_menu_app);
    }
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
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}
