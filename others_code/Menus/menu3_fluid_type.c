// main_menu.c
#include "menu.h"
#include "data.h"
extern Parameters_t g_parameters;

int fluid_handler(void* parameter)
{
    int fluid_index = *(int*)parameter;
    g_parameters.fluid_type = fluid_index;
    g_parameters.is_saved = 1;
    // SaveParameters(&g_parameters);
    return 0;
}
int menu3_fluid_setting_handler(void * parameter) {
     // 主菜单项
    static const MenuItem fliuid_items[] = {
        {"1.Water", fluid_handler},
        {"2.OIL", fluid_handler}
    };
    
    // 主菜单结构
    Menu calsetting_menu = {
        .title = "Fluid Type",
        .items_count = sizeof(fliuid_items)/sizeof(fliuid_items[0]),
        .items = fliuid_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = g_parameters.fluid_type,
        .type  = 1
    };
    
    return display_menu(&calsetting_menu);
}