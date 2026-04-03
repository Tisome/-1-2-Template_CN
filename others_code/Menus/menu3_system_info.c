// main_menu.c
#include "menu.h"
extern uint16_t g_equipment_number[4];
int menu3_system_info_handler(void * parameter)
{
 // 静态缓冲区用于存储动态生成的序列号字符串
    static char serial_str[32]; // 确保缓冲区足够大
    static char serial_str_sec[32]; // 确保缓冲区足够大
    // 使用所有4个元素生成序列号字符串
    snprintf(serial_str, sizeof(serial_str), 
             "S/N:%04X-%04X", 
             g_equipment_number[0],
             g_equipment_number[1]);
        // 历史数据子菜单项
    static const MenuItem history_items[] = {
        {"Device tpye:RUF 100", NULL , " 设备型号:RUF 100 "},
        {serial_str, NULL , serial_str },
        {"Software version:v2.0", NULL, " 软件版本:v2.0 "},
    };
    
    // 历史数据菜单结构
    Menu history_menu = {
        .title[0] = "System info",
        .title[1] = " 系统信息 ",
        .items_count = sizeof(history_items)/sizeof(history_items[0]),
        .items = history_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&history_menu);
}