#include "bsp_key.h"
#include "elog.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

static const char *key_value_to_str(uint8_t key_value)
{
    switch (key_value)
    {
    case KEY1_PRESS:
        return "KEY1 short press";
    case KEY2_PRESS:
        return "KEY2 short press";
    case KEY3_PRESS:
        return "KEY3 short press";
    case KEY4_PRESS:
        return "KEY4 short press";
    case KEY1_LONG_PRESS:
        return "KEY1 long press";
    case KEY2_LONG_PRESS:
        return "KEY2 long press";
    case KEY3_LONG_PRESS:
        return "KEY3 long press";
    case KEY4_LONG_PRESS:
        return "KEY4 long press";
    default:
        return "UNKNOWN";
    }
}

void task_key_test(void *parameter)
{
    (void)parameter;

    log_i("task_key_test start");
    log_i("Press KEY1~KEY4: short >= %d ms, long >= %d ms",
          (int)SHORT_PRESS_THRESHOLD_MS,
          (int)LONG_PRESS_THRESHOLD_MS);
    // usart_send_text_line("task_key_test start");
    // usart_send_text_line("Press KEY1~KEY4, watch key event output...");

    for (;;)
    {
        uint8_t key_value = key_scan(100);
        // char event_buf[96];
        // int event_len;

        if (key_value == KEY_NONE)
        {
            continue;
        }

        log_i("key event: %s (%u)", key_value_to_str(key_value), key_value);

        // event_len = snprintf(event_buf,
        //                      sizeof(event_buf),
        //                      "key event: %s (%u)",
        //                      key_value_to_str(key_value),
        //                      key_value);
        // if (event_len > 0)
        // {
        //     usart_send_text_line(event_buf);
        // }
    }
}
