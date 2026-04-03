#include "data.h"
#include "menu.h"

extern void flowout_config();
int output_mode_handler(void* parameter)
{
    int output_mode_index = *(int*)parameter;
    g_parameters.output_mode = output_mode_index;
    flowout_config();
    g_parameters.is_saved = 1;
    SaveParameters(&g_parameters);
    return 0;
}

int menu3_output_mode_handler(void * parameter) {
     // 主菜单项
    static const MenuItem fliuid_items[] = {
        {"4~20 mA", output_mode_handler, " 4~20 毫安 "},
        {"PNP", output_mode_handler , " PNP "},
        {"NPN", output_mode_handler, " NPN "}
    };
    
    // 主菜单结构
    Menu calsetting_menu = {
        .title[0] = "Output Mode",
        .title[1] = " 输出模式 ",
        .items_count = sizeof(fliuid_items)/sizeof(fliuid_items[0]),
        .items = fliuid_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = g_parameters.output_mode,
        .type  = 1
    };
    
    return display_menu(&calsetting_menu);
}
