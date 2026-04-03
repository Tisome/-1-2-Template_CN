// main_menu.c
#include "menu.h"
#include "data.h"
int g_display_index = 0;

void update_display_rate()
{
    switch(g_display_index)
    {
        case 0:
            g_parameters.sensitivity = 2;
            break;
        case 1:
            g_parameters.sensitivity = 5;
            break;
        case 2:
            g_parameters.sensitivity = 10;
            break;
     }
                
}
int display_rate_handler(void* parameter)
{
    int pip_index = *(int*)parameter;
    g_display_index = pip_index;
    update_display_rate();
    return 0;
}
int menu1_display_rate_handler(void * parameter) {
    // 历史数据子菜单项
    static const MenuItem history_items[] = {
        {"Low", display_rate_handler, " 低 "},
        {"Mid", display_rate_handler, " 中 "},
        {"High", display_rate_handler, " 高 "}
    };
    
    // 历史数据菜单结构
    Menu history_menu = {
        .title[0] = "sensitivity",
        .title[1] = " 灵敏度 ",
        .items_count = sizeof(history_items)/sizeof(history_items[0]),
        .items = history_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = g_display_index,
        .type  = 1
    };
    
    return display_menu(&history_menu);
}