// main_menu.c
#include "menu.h"
#include "data.h"
#include "bsp_key.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "math.h"
#include <stdio.h>
#include "st7789_reg_define.h"
#include "bsp_st7789_driver.h"
#include "bsp_gp22_handler.h"
#include "lv_ui.h"

extern bsp_st7789_driver_t st7789_driver_instance;
// 单位转换系数定义
#define M3_TO_L  (1000.0)
#define M3_TO_ML (1000000.0)
#define M3H_TO_LMIN (1000.0 / 60.0)
#define M3H_TO_MLMIN (1000000.0 / 60.0)
#define M3H_TO_LH (1000.0)
extern double g_flow_rate; //瞬时流速
extern double g_flow; //瞬时流量
extern double g_flow_info;    ///上电至今的流量

//历史流量数据
extern double empty;//占空比数值
extern Parameters_t g_parameters;
// 全局配置
DisplayMode current_mode = FLOW_RATE_MODE;
extern DisplayConfig mode_config[DISPLAY_MODE_COUNT];


//显示平均下来的流速值，使得显示更平滑
#define AVERAGE_BUFFER_SIZE 10
static double flow_rate_buffer[AVERAGE_BUFFER_SIZE] = {0};
static int buffer_index = 0;
static int buffer_count = 0;

extern double flow_rate_raw;
double static round_to_decimal(double num, int decimal_places) {
    double factor = pow(10, decimal_places);
    return round(num * factor) / factor;
}

// 平均值计算函数
static double calculate_average(double* buffer, double new_value, int* buffer_index, int *buffer_count) {
    // 添加新值到缓冲区
    buffer[*buffer_index] = new_value;
    
    //  如果缓冲区已满，移除最旧的值
    *buffer_index = (*buffer_index + 1) % AVERAGE_BUFFER_SIZE;
    *buffer_count = (*buffer_count ) % AVERAGE_BUFFER_SIZE + 1;
    // 计算平均值
    double sum = 0;
    for (int i = 0; i < *buffer_count; i++) {
        sum += buffer[i];
    }
    
    return (sum / *buffer_count);
}

// 显示模式固定为3个小数点前面的，一个小数点，将个小数点后面的
static void format_double_fixed_width_3_1_25(double* val, char *buffer, size_t buffer_size) 
{
    double num = *val;
    if (num < 0.0) num = 0.0;  // 处理负值
    if (num > 999.995) num = 999.99;  // 限制最大值

    // 四舍五入到百分位
    int32_t value = (int32_t)(num * 100.0 + 0.5);
    if (value > 99999) value = 99999;  // 确保不溢出

    // 提取整数和小数部分
    int32_t integer_part = value / 100;
    int32_t fractional_part = value % 100;

    // 格式化为固定6字符：XXX.XX
    snprintf(buffer, buffer_size, "%03d.%02d", 
         (int)integer_part, (int)fractional_part);
}

// 智能格式化函数：保证输出不超过8个字符
static void format_double_fixed_width(double* val, char *buffer, size_t buffer_size) 
{
        // 数值过大时显示"******"
    if (*val >= 99999999 || *val <= -99999999) {
         strncpy(buffer, "99999999", buffer_size);
        *val = 0;
        return;
    }
    
    double value = *val;
    // 根据数值大小动态确定小数点位置
    int precision;
    if (fabs(value) >= 1000000.0) {
        precision = 0;  // 整数部分7位，小数1位
    }else if (fabs(value) >= 100000.0) {
        precision = 1;  // 整数部分6位，小数1位
    }else if (fabs(value) >= 10000.0) {
        precision = 2;  // 整数部分5位，小数2位
    } else if (fabs(value) >= 1000.0) {
        precision = 3;  // 整数部分4位，小数3位
    } else if (fabs(value) >= 100.0) {
        precision = 3;  // 整数部分3位，小数4位
    } else if (fabs(value) >= 10.0) {
        precision = 5;  // 整数部分2位，小数5位
    } else if (fabs(value) >= 1.0) {
        precision = 6;  // 整数部分1位，小数6位
    } else {
        precision = 6;  // 纯小数，7位小数
    }

    // 格式化字符串
    char format_str[16];
    snprintf(format_str, sizeof(format_str), "%%.%df", precision);
    
    char temp_buf[16];
    snprintf(temp_buf, sizeof(temp_buf), format_str, value);
    
    // 移除多余的小数点（如果存在）
    char *decimal_point = strchr(temp_buf, '.');
    if (decimal_point) {
        // 移除尾部多余的零
//        char *ptr = temp_buf + strlen(temp_buf) - 1;
//        while (ptr > decimal_point && *ptr == '0') {
//            *ptr-- = '\0';
//        }
        
        // 如果小数点后没有数字，移除小数点
        if (*(decimal_point + 1) == '\0') {
            *decimal_point = '\0';
        }
    }

    // 确保总长度不超过8
    size_t len = strlen(temp_buf);
    if (len <= 8) {
        strncpy(buffer, temp_buf, buffer_size);
    } else {
        // 尝试减少精度
        for (int p = precision - 1; p >= 0; p--) {
            snprintf(format_str, sizeof(format_str), "%%.%df", p);
            snprintf(temp_buf, sizeof(temp_buf), format_str, value);
            
            // 移除多余的零
            decimal_point = strchr(temp_buf, '.');
            if (decimal_point) {
                char *ptr = temp_buf + strlen(temp_buf) - 1;
//                while (ptr > decimal_point && *ptr == '0') {
//                    *ptr-- = '\0';
//                }
//                if (*(decimal_point + 1) == '\0') {
//                    *decimal_point = '\0';
//                }
            }
            
            if (strlen(temp_buf) <= 6) {
                strncpy(buffer, temp_buf, buffer_size);
                return;
            }
        }
        
        // 仍然超长，显示星号
        strncpy(buffer, "9999999", buffer_size);
    }
}
// 根据设置的单位转换值
static double convert_value(double raw_value, int unit_type, int is_flow_rate) {
    switch (unit_type) {
        case 0: // m3 或 m3/h
            return raw_value;
            
        case 1: // L 或 L/min
            if (is_flow_rate) {
                return raw_value * M3H_TO_LMIN; // m3/h -> L/min
            } else {
                return raw_value * M3_TO_L; // m3 -> L
            }
            
        case 2: // mL 或 mL/min
            if (is_flow_rate) {
                return raw_value * M3H_TO_MLMIN; // m3/h -> mL/min
            } else {
                return raw_value * M3_TO_ML; // m3 -> mL
            }
            
        case 3: // L/h (仅流量)
            return raw_value * M3H_TO_LH; // m3/h -> L/h
            
        default:
            return raw_value;
    }
}
// 计算实际流速（考虑偏移）
static void calculate_true_flow_rate(double* display_flow_rate, double* display_flow_info, 
                                     DisplayConfig* current, int show_converted_units) {
    double temp_flow = 0;
    double temp_flow_info = 0;
    
    if (display_flow_rate != NULL) {
        // 流速模式
        temp_flow = *current->value;
        temp_flow_info = *current->secondary_value;
        
        
        // 需要转换单位
        if (show_converted_units) {
            // 流量转换为用户设置的单位
            temp_flow_info = convert_value(temp_flow_info, g_parameters.total_unit, 1);
            temp_flow      = convert_value(temp_flow, g_parameters.flow_rate_unit, 1);
        }
        
        *display_flow_rate = temp_flow;
        *display_flow_info = temp_flow_info;
    } else {
        // 历史流量模式
        temp_flow_info = *current->value;
        
        // 需要转换单位
        if (show_converted_units) {
            // 总量转换为用户设置的单位
            temp_flow_info = convert_value(temp_flow_info, g_parameters.total_unit, 0);
        }
        
        *display_flow_info = temp_flow_info;
    }
}


// 获取单位字符串
static const char* get_unit_string(int unit_type, int is_flow_rate) {
    switch (unit_type) {
        case 0: 
            return is_flow_rate ? "m3/h" : "m3";
        case 1: 
            return is_flow_rate ? "L/min" : "L";
        case 2: 
            return is_flow_rate ? "mL/min" : "mL";
        case 3: 
            return is_flow_rate ? "L/h" : "L"; // 对于总量单位，如果没有L/h选项，使用L
        default:
            return is_flow_rate ? "m3/h" : "m3";
    }
}
// 主显示函数
void display_flow_screen_zero(void) {
    ui_menu_all_disappear();
    char time_part[9], date_part[11];
    DisplayConfig* config = &mode_config[current_mode];
    double* display_flow_rate  = malloc(sizeof(double));
    double* display_flow_info  = malloc(sizeof(double));
    ui_display_all_appear();
    
    while(1) 
        {
            // 获取当前单位设置（每次循环都获取，确保单位改变后立即生效）
            const char* flow_rate_unit_str = get_unit_string(g_parameters.flow_rate_unit, 1);
            const char* total_unit_str = get_unit_string(g_parameters.total_unit, 0);
            config = &mode_config[current_mode];

            // 特殊处理流速模式   
            calculate_true_flow_rate(display_flow_rate,display_flow_info,config,1); 
            double flow_rate_avg = calculate_average(flow_rate_buffer, *display_flow_rate, &buffer_index, &buffer_count);
            double true_flow_rate_avg = round_to_decimal(flow_rate_avg, 2);         
           // 显示主值
            char value_str[10];
            format_double_fixed_width_3_1_25(&true_flow_rate_avg, value_str, sizeof(value_str));
            char secondary_str[10];
            format_double_fixed_width(display_flow_info, secondary_str, sizeof(secondary_str));        


            ui_display_set_value(value_str,flow_rate_unit_str,secondary_str,total_unit_str,
                                            doubleToString(g_GP22_data.empty_tube, 2),NULL);
        
        // 按键处理
        static uint8_t KeyNum;
        KeyNum = key_scan(0);
        if(KeyNum == KEY3_PRES) 
        { // 返回
            free(display_flow_rate);
            free(display_flow_info);
            ui_display_all_disappear();
            ui_menu_all_appear();
            return;
        }
        else if(KeyNum == KEY2_PRES) 
        { // 清零
            g_parameters.offset = flow_rate_raw;
            SaveParameters(&g_parameters);
        }
        
        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

int menu3_set_zero_handler(void * parameter) {
    display_flow_screen_zero();
    
    return 0;
}