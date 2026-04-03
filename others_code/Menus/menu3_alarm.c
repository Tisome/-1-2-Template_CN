// main_menu.c
#include "menu.h"
#include "data.h"
#include "parameter_editor.h"
int menu3_alarm_handler(void * parameter) {
    // 主菜单项
    static const MenuItem main_items[] = {
        {"Upper", menu4_alarm_upper_range_handler, " 上限 "},
        {"Lower", menu4_alarm_lower_range_handler, " 下限 "},
    };
    
    // 主菜单结构
    Menu main_menu = {
        .title[0] = "Alarm range",
        .title[1] = " 报警范围 ",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&main_menu);
}