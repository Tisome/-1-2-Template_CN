// main_menu.c
#include "menu.h"

int menu1_history_handler(void * parameter) {
    // 鍘嗗彶鏁版嵁瀛愯彍鍗曢」
    static const MenuItem history_items[] = {
        {"1.By Day", menu2_history_byday_handler},
        {"2.By Month", menu2_history_bymonth_handler},
        {"3.By Year", menu2_history_byyear_handler},
        {"4.Total", menu2_history_total_handler}
    };
    
    // 鍘嗗彶鏁版嵁鑿滃崟缁撴瀯
    Menu history_menu = {
        .title = "History data menu",
        .items_count = sizeof(history_items)/sizeof(history_items[0]),
        .items = history_items,
        .items_per_page = DEFAULT_ITEMS_PER_PAGE,
        .start = 0
    };
    
    return display_menu(&history_menu);
}