// main_menu.c
#include "menu.h"

int menu2_unitsetting_handler(void * parameter) {
    // 主菜单项
    static const MenuItem main_items[] = {
        {"Flow rate unit", menu3_flow_rate_unit_handler, " 流量单位 "},
        {"Total unit", menu3_total_unit_handler, " 累计量单位 "},
    };
    
    // 主菜单结构
    Menu sys_menu = {
        .title[0] = "System Setting Menu",
        .title[1] = " 单位设置菜单 ",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&sys_menu);
}