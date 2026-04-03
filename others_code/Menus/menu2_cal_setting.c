// main_menu.c
#include "menu.h"
#include "parameter_editor.h"
int menu3_zero_cut_handler(void * parameter) {
    return menu3_outer_diameterSetting();
}
int menu2_calsetting_handler(void * parameter) {
    // 主菜单项
    static const MenuItem main_items[] = {
        {"K Factor", menu3_scale_factor_handler," 校准系数 "},
        {"Zero Cut", menu3_zero_cutSetting," 零切下限 "},
        // {"4~20mA CAL", menu3_4_20_ma_handler," 4~20mA 校准 "},
        {"Zero CAL", menu3_set_zero_handler," 零点校准 "}
    };
    
    // 主菜单结构
    Menu calsetting_menu = {
        .title[0] = "Calibration",
        .title[1] = "校准",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&calsetting_menu);
}
