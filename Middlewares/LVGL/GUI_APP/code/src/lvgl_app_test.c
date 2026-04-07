/*
 * LVGL 任务适配文件。
 * 当前只保留一个很薄的封装层，把测试任务入口 `task_lvgl_test()` 转发到真正的
 * 菜单应用任务 `menu_app_task()`，便于任务创建层与界面实现层解耦。
 */
#include "lvgl_app_test.h"

#include "menu_app.h"

/* LVGL 测试任务入口，实际工作由 `menu_app_task()` 完成。 */
void task_lvgl_test(void *p)
{
    menu_app_task(p);
}
