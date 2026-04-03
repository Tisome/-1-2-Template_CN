// main_menu.c
#include "menu.h"
#include "data.h"

// extern Parameters_t g_parameters;

int menu4_flow_rate_unit_handler(void* parameter)
{
    int pip_index = *(int*)parameter;
    g_parameters.flow_rate_unit = pip_index;
    g_parameters.is_saved = 1;
    SaveParameters(&g_parameters);
    return 0;
}
int menu3_flow_rate_unit_handler(void * parameter) {
     // 材质设置
    static const MenuItem flow_rate_unit_items[] = {
        {"m3/h", menu4_flow_rate_unit_handler, " 立方米每小时 "},
        {"L/min", menu4_flow_rate_unit_handler, " 升每分钟 "},
        {"mL/min", menu4_flow_rate_unit_handler," 毫升每分钟 "},
        {"L/h", menu4_flow_rate_unit_handler, " 升每小时 "}
    };
    
    // 主菜单结构
    Menu FlowrateSetting_menu = {
        .title[0] = "Flow rate unit",
        .title[1] = " 流量单位 ",
        .items_count = sizeof(flow_rate_unit_items)/sizeof(flow_rate_unit_items[0]),
        .items = flow_rate_unit_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start =g_parameters.flow_rate_unit,
        .type  = 1
    };
    
    return display_menu(&FlowrateSetting_menu);
}