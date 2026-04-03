// main_menu.c
#include "menu.h"
#include "data.h"

// extern Parameters_t g_parameters;

int menu4_total_unit_handler(void* parameter)
{
    int pip_index = *(int*)parameter;
    g_parameters.total_unit = pip_index;
    SaveParameters(&g_parameters);
    return 0;
}
int menu3_total_unit_handler(void * parameter) {
     // 材质设置
    static const MenuItem total_unit_items[] = {
        {"m3", menu4_total_unit_handler, " 立方米 "},
        {"L", menu4_total_unit_handler, " 升 "},
        {"mL", menu4_total_unit_handler, " 毫升 "}
    };
    
    // 主菜单结构
    Menu FlowrateSetting_menu = {
        .title[0] = "Total unit",
        .title[1] = " 累计量单位 ",
        .items_count = sizeof(total_unit_items)/sizeof(total_unit_items[0]),
        .items = total_unit_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start =g_parameters.total_unit,
        .type  = 1
    };
    
    return display_menu(&FlowrateSetting_menu);
}

