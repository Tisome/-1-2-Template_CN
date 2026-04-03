// main_menu.c
#include "menu.h"
#include "data.h"
int menu1_setting_language(void * parameter);
int menu2_syssetting_handler(void * parameter) {
    // 主菜单项
    static const MenuItem main_items[] = {
        {"Language", menu1_setting_language," 语言设置 "},
        {"Sensitivity", menu1_display_rate_handler,"  灵敏度 "},
        {"Screen Rotation", menu3_display_direction_handler," 屏幕旋转 "},
        {"System INFO", menu3_system_info_handler, " 产品信息 "}
    };
    
    // 主菜单结构
    Menu sys_menu = {
        .title[0] = "System Setting Menu",
        .title[1] = " 系统设置菜单 ",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&sys_menu);
}

int language_handler(void* parameter)
{
    int pip_index = *(int*)parameter;
    g_parameters.language = pip_index;
    g_parameters.is_saved = 1;
    SaveParameters(&g_parameters);
    return 0;
}
int menu1_setting_language(void * parameter) {
    // 设置子菜单项
    static const MenuItem setting_items[] = {
        {"English", language_handler," 英文 "},
        {"Chinese", language_handler," 中文 "},

    };
    
    // 设置菜单结构
    Menu setting_menu = {
        .title[0] = "Setup menu",
        .title[1] = " 设置菜单 ",
        .items_count = sizeof(setting_items)/sizeof(setting_items[0]),
        .items = setting_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = g_parameters.language
    };
    
    return display_menu(&setting_menu);
}

