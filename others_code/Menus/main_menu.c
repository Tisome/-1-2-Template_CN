// main_menu.c
#include "menu.h"
#include "data.h"


int menu1_setting_Zero(void*);
/*  测量显示设置语言主菜单管道参数系统校准输出方向帧率范围模脉冲系材质流量单
位累计信息下限上左右每小时升毫克立方米塑料金属合钢从机地址波特字长验停止位型
号软件版本外径壁厚秒备比例因子归零式安转中文英*/
int main_menu_handler(void * parameter) {
    // 主菜单项
    static const MenuItem main_items[] = {
        {"Measurements", menu1_start_handler," 测量显示 "},
        // {"Setting", menu1_setting_handler," 设置 "},
        // {"Language", menu1_setting_language," 语言 "},
        {"Pipe Setting", menu2_pipsetting_handler," 管道设置 "},
        {"Output Setting", menu2_outsetting_handler," 输出设置 "},
        {"Unit Setting", menu2_unitsetting_handler," 单位设置 "},
        {"CAL Setting", menu2_calsetting_handler," 校准设置 "},
        {"SYS Setting", menu2_syssetting_handler," 系统设置 "},
    };
    
    // 主菜单结构
    Menu main_menu = {
        .title[0] = "Main Menu",
        .title[1] = " 主菜单 ",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0,
    };
    
    return display_menu(&main_menu);
}


