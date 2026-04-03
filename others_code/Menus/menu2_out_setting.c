// main_menu.c
#include "menu.h"
#include "data.h"
#include "parameter_editor.h"


int menu4_pulse_per_flow_handler(void * parameter) {
    return menu4_pulse_per_flowSetting(NULL);
}
int menu2_outsetting_handler(void * parameter) {
    // 主菜单项
    static const MenuItem main_items[] = 
    {
        {"Modbus ADDR", menu4_modbus_addr_handler , " Modbus地址 "},
        {"Output mode"   , menu3_output_mode_handler, " 输出模式 "    },
        {"PLS K-factor", menu4_pulse_per_flow_handler, " 脉冲K-系数 " },
        // {"Rs485 setup"   , menu3_rs485_setting_handler," 485设置 "  },
        {"Analog Output"  , menu3_4_20_mh_handler, " 模拟输出 "  },
        {"ALM Output"  , menu3_alarm_handler, " 报警输出 "  },
    };
    
    // 主菜单结构
    Menu outsetting_menu = {
        .title[0] = "Output Setting",
        .title[1] = " 输出设置 ",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&outsetting_menu);
}
