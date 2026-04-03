// main_menu.c
#include "menu.h"
#include "data.h"
#include "data.h"
#include "dac.h"
#include "bsp_gp22_handler.h"
#include "elog.h"
#include "link_total.h"
#include "bsp_st7789_driver.h"
// 定义结构体存储不同材料的配置

int display_direaction_handler(void* parameter)
{
    static int last_state = 0;
    int pip_index = *(int*)parameter;
    g_parameters.rotate_state = (last_state - 2 * pip_index + 1 + 4) % 4;
    st7789_rotate(&st7789_driver_instance,g_parameters.rotate_state);
    last_state = g_parameters.rotate_state;
    SaveParameters(&g_parameters);
    return 0;
}
int menu3_display_direction_handler(void * parameter) {
     // 材质设置
    static const MenuItem fliuid_items[] = {
        {"RIGHT", display_direaction_handler, " 右转 "},
        {"LEFT", display_direaction_handler, " 左转 "}
    };
    
    // 主菜单结构
    Menu Materialsetting_menu = {
        .title[0] = "Display Direction",
        .title[1] = " 显示方向 ",
        .items_count = sizeof(fliuid_items)/sizeof(fliuid_items[0]),
        .items = fliuid_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start =0,
        .type  = 0
    };
    
    return display_menu(&Materialsetting_menu);
}