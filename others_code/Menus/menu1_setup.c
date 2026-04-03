// main_menu.c
#include "menu.h"

int menu1_setting_handler(void * parameter) {
    // 设置子菜单项
    static const MenuItem setting_items[] = {
        {"Pipe parameter", menu2_pipsetting_handler," 管道参数 "},
        {"System setting", menu2_syssetting_handler," 系统设置 "},
        {"Calibration", menu2_calsetting_handler," 校准设置 "},
        {"Output setting", menu2_outsetting_handler," 输出设置 "},
        {"Display setting", menu2_displaysetting_handler," 显示设置 "}

    };
    
    // 设置菜单结构
    Menu setting_menu = {
        .title[0] = "Setup menu",
        .title[1] = " 设置菜单 ",
        .items_count = sizeof(setting_items)/sizeof(setting_items[0]),
        .items = setting_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&setting_menu);
}