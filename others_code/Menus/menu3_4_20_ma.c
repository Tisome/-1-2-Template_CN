// main_menu.c
#include "menu.h"
#include "elog.h"
//count=(I(ma)/25)*10000+170-3I
void mA_Start(void)
{
    // HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
void mA_Stop(void)
{
    // HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}
// 设置输出电流值 (4.0 - 20.0 mA)
void set_output_current(float current_ma)
{
    // 电流范围限制
    if (current_ma < 4.0f) current_ma = 4.0f;
    if (current_ma > 20.0f) current_ma = 20.0f;
    
    // 计算所需电压 (基于XTR111特性)
    float voltage = (current_ma - 4.0f) * 5.0f;
    
    
    // 计算PWM占空比 (假设MCU工作电压3.3V) 4000对应百分之50
    int ma_cycle = current_ma*394 - 230;
    log_d("Set Current: %.2f mA, Voltage: %.2f V, PWM Cycle: %d", 
                                  current_ma, voltage, ma_cycle);
    // 更新PWM占空比
    // __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ma_cycle);
}
// int menu4_4_ma_handler(void * parameter) 
// {
//     set_output_current(4);
//     return 0;
// }
// int menu4_8_ma_handler(void * parameter) 
// {
//     set_output_current(8);
//     return 0;
// }
// int menu4_12_ma_handler(void * parameter)
// {
//     set_output_current(12);
//     return 0;
// }
// int menu4_16_ma_handler(void * parameter) 
// {
//     set_output_current(16);
//     return 0;
// }
// int menu4_20_ma_handler(void * parameter) {
//     set_output_current(20);
//     return 0;
// }
int menu3_4_20_ma_handler(void * parameter) 
{
    mA_Start();
     // 材质设置
    static const MenuItem fliuid_items[] = {
        // {"1.4 mA",   menu4_4_ma_handler},
        // {"2.8 mA",   menu4_8_ma_handler},
        // {"3.12 mA", menu4_12_ma_handler},
        // {"4.16 mA", menu4_16_ma_handler},
        // {"5.20 mA", menu4_20_ma_handler},
        {"4 mA",   NULL," 4 毫安 "},
        {"8 mA",   NULL, " 8 毫安 "},
        {"12 mA", NULL, " 12 毫安 "},
        {"16 mA", NULL, " 16 毫安 "},
        {"20 mA", NULL, " 20 毫安 "},
    };
    
    // 主菜单结构
    Menu Materialsetting_menu = {
        .title[0] = "4~20 mA CAL",
        .title[1] = " 4~20 毫安 校准 ",
        .items_count = sizeof(fliuid_items)/sizeof(fliuid_items[0]),
        .items = fliuid_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start =0,
        .type  = 1
    };
    
    return display_menu(&Materialsetting_menu);
}