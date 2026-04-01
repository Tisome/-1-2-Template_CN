#include "lvgl_app_test.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"

static lv_obj_t *s_tick_label = NULL;

static void lvgl_test_timer_cb(lv_timer_t *timer)
{
    static uint32_t tick_count = 0U;

    LV_UNUSED(timer);

    tick_count++;

    if (s_tick_label != NULL)
    {
        lv_label_set_text_fmt(s_tick_label, "tick %lu", (unsigned long)tick_count);
    }
}

static void lvgl_test_create_ui(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *panel;
    lv_obj_t *title;
    lv_obj_t *subtitle;

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    panel = lv_obj_create(scr);
    lv_obj_set_size(panel, 200, 120);
    lv_obj_center(panel);
    lv_obj_set_style_radius(panel, 12, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1B998B), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 12, 0);

    title = lv_label_create(panel);
    lv_label_set_text(title, "LVGL LCD TEST");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    subtitle = lv_label_create(panel);
    lv_label_set_text(subtitle, "display is running");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xEAF4F4), 0);
    lv_obj_align(subtitle, LV_ALIGN_CENTER, 0, 4);

    s_tick_label = lv_label_create(panel);
    lv_label_set_text(s_tick_label, "tick 0");
    lv_obj_set_style_text_color(s_tick_label, lv_color_hex(0x0B132B), 0);
    lv_obj_align(s_tick_label, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_timer_create(lvgl_test_timer_cb, 500U, NULL);
}

void task_lvgl_test(void *p)
{
    (void)p;

    lv_init();
    lv_port_disp_init();
    lvgl_test_create_ui();

    while (1)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}
