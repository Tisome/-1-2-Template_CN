// main_menu.c
#include "menu.h"
#include "data.h"
#include "parameter_editor.h"
int menu3_modbus_setting_handler(void * parameter) {
    // 历史数据子菜单项
    static const MenuItem history_items[] = {
        {"Slave addr", menu4_modbus_addr_handler , " 从机地址 "},
    };
    
    // 历史数据菜单结构
    Menu history_menu = {
        .title[0] = "Modbus Setting",
        .title[1] = " Modbus设置 ",
        .items_count = sizeof(history_items)/sizeof(history_items[0]),
        .items = history_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&history_menu);
}
