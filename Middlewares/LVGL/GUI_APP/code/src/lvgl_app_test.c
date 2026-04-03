#include "lvgl_app_test.h"

#include "menu_app.h"

void task_lvgl_test(void *p)
{
    menu_app_task(p);
}
