// main_menu.c
#include "menu.h"

int menu2_pipsetting_handler(void * parameter) 
{
    // 主菜单项
    static const MenuItem main_items[] = {
        {"Outer diameter", menu3_outer_diameter_handler, " 管道外径 "},
        {"Wall thickness", menu3_wall_thickmess_handler, " 管道壁厚 "},
        {"Material", menu3_material_handler, " 管道材质 "},
    };
    
    // 主菜单结构
    Menu pipe_parameter_menu = {
        .title[0] = "Pipe parameter",
        .title[1] = " 管道设置 ",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&pipe_parameter_menu);
}