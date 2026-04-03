#include "main.h"
#include "stdio.h"
#include "menu.h"
#include "bsp_key.h"
/* RTC闁跨喐鏋婚幏鐑芥晸閻欌剝鎷濋幏鐑芥晸閿燂拷 */


uint8_t IsLeapYear(uint8_t year);
int time_setting(void) {
    // 闁跨喐鏋婚幏宄板闁跨喐鏋婚幏鐑芥晸閻偆顣幏閿嬫闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐕傛嫹
    uint8_t current_hour = g_currentTime.Hours;
    uint8_t current_minute = g_currentTime.Minutes;
    uint8_t current_second = g_currentTime.Seconds;
    // 閻樿埖鈧線鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
    typedef enum {SET_HOUR, SET_MINUTE, SET_SECOND} TimeSetState;
    TimeSetState state = SET_HOUR;
    
    // 闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
    uint32_t last_blink_time = 0;
    uint8_t blink_state = 1; // 1闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏锟�0闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏鐑芥晸閹搭亷缍囬幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗閻戜焦鏅ラ柨鐔告灮閹风兘鏁撻弬銈嗗
    const uint32_t BLINK_INTERVAL = 200; // 闁跨喐鏋婚幏椋庡剨闁跨喐鏋婚幏鐑芥晸缁差槞
    
    // 闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏宄邦潗閺冨爼鏁撻弬銈嗗
    char time_str[20];
    // sprintf(time_str, "%02d:%02d:%02d", current_hour, current_minute, current_second);
    OLED_ShowString(30, 30, (uint8_t*)time_str, 16);
    oled_refresh_gram();
    
    while (1) {
        // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗
        uint8_t key_val = key_scan(0);
        
        switch (key_val) {
            case KEY1_PRES: // UP闁跨喐鏋婚幏锟� - 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹峰嘲鈧拷
                switch (state) {
                    case SET_HOUR:
                        current_hour = (current_hour == 0) ? 23 : current_hour - 1;
                        break;
                    case SET_MINUTE:
                        current_minute = (current_minute == 0) ? 59 : current_minute - 1;
                        break;
                    case SET_SECOND:
                        current_second = (current_second == 0) ? 59 : current_second - 1;
                        break;
                }
                break;
                
            case KEY0_PRES: // DOWN闁跨喐鏋婚幏锟� - 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹峰嘲鈧拷
                switch (state) {
                    case SET_HOUR:
                        current_hour = (current_hour + 1) % 24;
                        break;
                    case SET_MINUTE:
                        current_minute = (current_minute + 1) % 60;
                        break;
                    case SET_SECOND:
                        current_second = (current_second + 1) % 60;
                        break;
                }
                break;
                
            case KEY2_PRES: // ENTER闁跨喐鏋婚幏锟� - 闁跨喎褰ㄦ导娆愬闁跨喐鏋婚幏鐑芥晸閺傘倖瀚规稉鈧悩鑸碘偓渚€鏁撻弬銈嗗绾噣鏁撻弬銈嗗
                  state = (TimeSetState)((state + 1) % 3);
                break;
                
            case KEY3_PRES: // 绾噣鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閻偓缍囬幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风úTC
            {
                    // RTC_TimeTypeDef newTime = {
                    //     .Hours = current_hour,
                    //     .Minutes = current_minute,
                    //     .Seconds = current_second,
                    // };
                    // HAL_RTC_SetTime(&hrtc, &newTime, RTC_FORMAT_BIN); 
                    // OLED_Clear();
                    return 0;
                }
        }
        
        // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹烽鍎婇弫鍫ユ晸閺傘倖瀚�
        if (HAL_GetTick() - last_blink_time > BLINK_INTERVAL) {
            blink_state = !blink_state;
            last_blink_time = HAL_GetTick();
        }
        
        
            // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹烽鍎婇悩鑸碘偓渚€鏁撻柊鐢殿暜閹峰嘲澧犻柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚圭粈锟�
            switch (state) {
                case SET_HOUR:
                    if (blink_state) {
                        sprintf(time_str, "  :%02d:%02d", current_minute, current_second);
                    } else {
                        sprintf(time_str, "%02d:%02d:%02d", current_hour, current_minute, current_second);
                    }
                    break;
                    
                case SET_MINUTE:
                    if (blink_state) {
                        sprintf(time_str, "%02d:  :%02d", current_hour, current_second);
                    } else {
                        sprintf(time_str, "%02d:%02d:%02d", current_hour, current_minute, current_second);
                    }
                    break;
                    
                case SET_SECOND:
                    if (blink_state) {
                        sprintf(time_str, "%02d:%02d:  ", current_hour, current_minute);
                    } else {
                        sprintf(time_str, "%02d:%02d:%02d", current_hour, current_minute, current_second);
                    }
                    break;
                    
                default:
                    sprintf(time_str, "%02d:%02d:%02d", current_hour, current_minute, current_second);
                    break;
            }
            OLED_ShowString(30, 30, (uint8_t*)time_str, 16);
        
        // 闁跨喐鏋婚幏椋庛仛閻樿埖鈧焦瀵氱粈锟�
        const char* state_text = "";
        switch (state) {
            case SET_HOUR: state_text = "Set Hour   "; break;
            case SET_MINUTE: state_text = "Set Minute  " ; break;
            case SET_SECOND: state_text = "Set Second   "; break;
        }
        OLED_ShowString(30, 0, (uint8_t*)state_text, 16);
        
        // 閸掔兘鏁撻弬銈嗗闁跨喐鏋婚幏椋庛仛
        oled_refresh_gram();
        
        // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹烽攱妞傞柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏绋U閸楃娀鏁撻惌顐ゆ閹风兘鏁撻弬銈嗗
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    return 0;
}
int date_setting(void) {
    // 闁跨喐鏋婚幏宄板闁跨喐鏋婚幏鐑芥晸閻偆顣幏閿嬫闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐕傛嫹
    int month_days[12] = {31,28,31,30,31,30,31,31,30,31,30,31}; 
    // uint8_t current_year = g_currentDate.Year;
    // uint8_t current_month = g_currentDate.Month;
    // uint8_t current_date = g_currentDate.Date;
    // 閻樿埖鈧線鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
    typedef enum {SET_HOUR, SET_MINUTE, SET_SECOND} TimeSetState;
    TimeSetState state = SET_HOUR;
    
    // 闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
    uint32_t last_blink_time = 0;
    uint8_t blink_state = 1; // 1闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏锟�0闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏鐑芥晸閹搭亷缍囬幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗閻戜焦鏅ラ柨鐔告灮閹风兘鏁撻弬銈嗗
    const uint32_t BLINK_INTERVAL = 200; // 闁跨喐鏋婚幏椋庡剨闁跨喐鏋婚幏鐑芥晸缁差槞
    
    // 闁跨喐鏋婚幏椋庛仛闁跨喐鏋婚幏宄邦潗閺冨爼鏁撻弬銈嗗
    char time_str[20];
    // sprintf(time_str, "%02d:%02d:%02d", current_year, current_month, current_date);
    OLED_ShowString(30, 30, (uint8_t*)time_str, 16);
    oled_refresh_gram();
    
    while (1) {
        // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗
        uint8_t key_val = key_scan(0);
        
        switch (key_val) {
            case KEY1_PRES: // UP闁跨喐鏋婚幏锟� - 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹峰嘲鈧拷
                switch (state) {
                    case SET_HOUR:
                        current_year = (current_year == 1) ? 99 : current_year - 1;
                        break;
                    case SET_MINUTE:
                        current_month = (current_month == 1) ? 12 : current_month - 1;
                        break;
                    case SET_SECOND:
                        current_date = (current_date == 1) ? month_days[current_month - 1] : current_date - 1;
                        break;
                }
                break;
                
            case KEY0_PRES: // DOWN闁跨喐鏋婚幏锟� - 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹峰嘲鈧拷
                switch (state) {
                    case SET_HOUR:
                        current_year = current_year ++;
                        break;
                    case SET_MINUTE:
                        current_month = (current_month ) % 12 + 1;
                        break;
                    case SET_SECOND:
                        current_date = (current_date ) % month_days[current_month - 1] + 1;
                        break;
                }
                break;
                
            case KEY2_PRES: // ENTER闁跨喐鏋婚幏锟� - 闁跨喎褰ㄦ导娆愬闁跨喐鏋婚幏鐑芥晸閺傘倖瀚规稉鈧悩鑸碘偓渚€鏁撻弬銈嗗绾噣鏁撻弬銈嗗
                  state = (TimeSetState)((state + 1) % 3);
                break;
                
            case KEY3_PRES: // 绾噣鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閻偓缍囬幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风úTC
            {
                    // RTC_DateTypeDef newTime = {
                    //     .Year = current_year,
                    //     .Month = current_month,
                    //     .Date = current_date,
                    // };
                    HAL_RTC_SetDate(&hrtc, &newTime, RTC_FORMAT_BIN); 
                    RTC_Backup_DateTime();
                    OLED_Clear();
                    return 0;
                }
        }
        
        // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹烽鍎婇弫鍫ユ晸閺傘倖瀚�
        if (HAL_GetTick() - last_blink_time > BLINK_INTERVAL) {
            blink_state = !blink_state;
            last_blink_time = HAL_GetTick();
        }
        
        
            // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹烽鍎婇悩鑸碘偓渚€鏁撻柊鐢殿暜閹峰嘲澧犻柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚圭粈锟�
            switch (state) {
                case SET_HOUR:
                    if (blink_state) {
                        sprintf(time_str, "20  :%02d:%02d", current_month, current_date);
                    } else {
                        sprintf(time_str, "20%02d:%02d:%02d", current_year, current_month, current_date);
                    }
                    break;
                    
                case SET_MINUTE:
                    if (blink_state) {
                        sprintf(time_str, "20%02d:  :%02d", current_year, current_date);
                    } else {
                        sprintf(time_str, "20%02d:%02d:%02d", current_year, current_month, current_date);
                    }
                    break;
                    
                case SET_SECOND:
                    if (blink_state) {
                        sprintf(time_str, "20%02d:%02d:  ", current_year, current_month);
                    } else {
                        sprintf(time_str, "20%02d:%02d:%02d", current_year, current_month, current_date);
                    }
                    break;
                    
                default:
                    sprintf(time_str, "20%02d:%02d:%02d", current_year, current_month, current_date);
                    break;
            }
            OLED_ShowString(20, 30, (uint8_t*)time_str, 16);
        
        // 闁跨喐鏋婚幏椋庛仛閻樿埖鈧焦瀵氱粈锟�
        const char* state_text = "";
        switch (state) {
            case SET_HOUR: state_text = "Set Year   "; break;
            case SET_MINUTE: state_text = "Set Month   "; break;
            case SET_SECOND: state_text = "Set Day    "; break;
        }
        OLED_ShowString(30, 0, (uint8_t*)state_text, 16);
        
        // 閸掔兘鏁撻弬銈嗗闁跨喐鏋婚幏椋庛仛
        oled_refresh_gram();
        
       if(IsLeapYear(current_year))
       {   
        month_days[1] = 29;
       }
       else
       {
        month_days[1] = 28;
       }
        // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹烽攱妞傞柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏绋U閸楃娀鏁撻惌顐ゆ閹风兘鏁撻弬銈嗗
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    return 0;
}
int menu4_time_set_ymd(void * parameter);
int menu4_time_set_hms(void * parameter);
// 闁跨喎澹欑喊澶嬪闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗
int menu3_time_set_handler(void * parameter) {
        static const MenuItem main_items[] = {
        {"1.Year-Month-Day", menu4_time_set_ymd},
        {"2.Hour-Min-Sec", menu4_time_set_hms}
    };
        // 闁跨喐鏋婚幏鐑芥晸閸撹法顣幏鐑芥晸缂佹挻鐎�
    Menu tim_setting_menu = {
        .title = "Time Setting",
        .items_count = sizeof(main_items)/sizeof(main_items[0]),
        .items = main_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&tim_setting_menu);
    
}

// 闁跨喎澹欑喊澶嬪闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗
int menu4_time_set_ymd(void * parameter) {
    date_setting();
    return 0;
    
}

int menu4_time_set_hms(void * parameter) {
    time_setting();
    
    return 0;
    
}

/**
  * @brief  闁跨喎褰ㄧ拋瑙勫閹稿洭鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚瑰▎鐘绘晸鏉炴浜烽幏鐑芥晸閺傘倖瀚归柨鐕傛嫹
  * @param  year: 闁跨喐鏋婚幏鐤春闁跨噦鎷�0-99闁跨喐鏋婚幏椋庛仛2000-2099闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐕傛嫹2025闁跨喐鏋婚幏锟�
  * @retval 1=闁跨喐鏋婚幏鐑芥晸閻ゃ儻绱�0=楠炴娊鏁撻弬銈嗗
  */
uint8_t IsLeapYear(uint8_t year) {
    // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�2000-2099闁跨喍鑼庣涵閿嬪閸愭瑩鏁撻弬銈嗗閼解晠鏁撶徊濂32 RTC闁跨喐鏋婚幏鐑芥晸閻偊娼婚幏宄扮础闁跨喐鏋婚幏锟�
    uint16_t full_year = (year < 100) ? (2000 + year) : year;
    
    // 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻敓锟�
    // 1. 闁跨喐婢冮幉瀣4闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔稿疆鐠囇勫闁跨喐婢冮幉瀣100闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗
    // 2. 闁跨喐婢冮幉瀣400闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
    return ((full_year % 4 == 0) && (full_year % 100 != 0)) || (full_year % 400 == 0);
}