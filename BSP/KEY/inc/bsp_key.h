#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"

#define KEY_NUMS                 (4)

#define KEY_NONE                 0 /* 没有按键按下 */
#define KEY1_PRESS               1 /* KEY1按下 */
#define KEY2_PRESS               2 /* KEY2按下 */
#define KEY3_PRESS               3 /* KEY3按下 */
#define KEY4_PRESS               4 /* KEY4按下 */
#define KEY1_LONG_PRESS          5 /* KEY1长按 */
#define KEY2_LONG_PRESS          6 /* KEY2长按 */
#define KEY3_LONG_PRESS          7 /* KEY3长按 */
#define KEY4_LONG_PRESS          8 /* KEY4长按 */

#define KEY_PRESS_GET_TIME_MS()  (xTaskGetTickCount() * portTICK_PERIOD_MS) /* 获取当前系统时间，单位为毫秒 */
#define DEBOUNCE_TIME_MS         50                                         /* 消抖时间，单位为毫秒 */
#define SHORT_PRESS_THRESHOLD_MS 80                                         /* 短按时间阈值，单位为毫秒 */
#define LONG_PRESS_THRESHOLD_MS  500                                        /* 长按时间阈值，单位*/

void key_gpio_exti_handler(uint16_t GPIO_Pin);
void task_key(void *parameter);
uint8_t key_scan(uint8_t timeout_ms);

extern TaskHandle_t task_key_handle;

typedef enum {
    KEY_OK              = 0,
    KEY_ERROR           = 1,
    KEY_BUSY            = 2,
    KEY_TIMEOUT         = 3,
    KEY_ERROR_RESOURCE  = 4,
    KEY_ERROR_PARAMETER = 5,
    KEY_ERROR_NO_MEMORY = 6,
    KEY_RESERVED        = 0xFF

} key_status_t;

typedef struct
{
    uint8_t event_index;         /* 事件索引，0表示下降沿事件，1表示上升沿事件 */
    uint32_t first_trigger_time; /* 首次触发时间，单位为毫秒 */
} key_runtime_state_t;

typedef enum {
    KEY_PRESSED       = 0,
    KEY_NOT_PRESSED   = 1,
    KEY_SHORT_PRESSED = 2,
    KEY_LONG_PRESSED  = 3
} key_press_status_t;

typedef enum {
    KEY_EDGE_NONE    = 0,
    KEY_RAISING_EDGE = 1,
    KEY_FALLING_EDGE = 2
} key_trigger_edge_t;

typedef enum {
    KEY_ID_1 = 0,
    KEY_ID_2 = 1,
    KEY_ID_3 = 2,
    KEY_ID_4 = 3,
    KEY_ID_NONE = 0xFF
} key_id_t;

typedef struct
{
    key_id_t key_id;                     /* 按键ID，取值范围为0-3，分别对应KEY1-KEY4 */
    key_trigger_edge_t key_trigger_edge; /* 触发边沿，0表示上升沿，1表示下降沿 */
    uint32_t key_trigger_time;           /* 按键触发时间，单位为毫秒 */
} key_press_event_t;

#endif /* __BSP_KEY_H */
