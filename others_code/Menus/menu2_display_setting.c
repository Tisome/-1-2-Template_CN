#include "menu.h"


int menu2_displaysetting_handler(void * parameter) {
    // 主菜单项
    static const MenuItem main_items[] = {
        {"Didplay direction", menu3_display_direction_handler," 显示方向 "},
        {"Display frame rate", menu1_display_rate_handler," 显示帧率 "},
    };
    
    // 主菜单结构
    Menu display_parameter_menu = {
        .title[0] = "Display Setting",
        .title[1] = " 显示设置 ",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&display_parameter_menu);
}