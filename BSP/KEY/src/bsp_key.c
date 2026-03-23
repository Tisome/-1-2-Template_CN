#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "bsp_key.h"
#include "elog.h"

static key_trigger_edge_t irq_type[KEY_NUMS] = {
    KEY_FALLING_EDGE,
    KEY_FALLING_EDGE,
    KEY_FALLING_EDGE,
    KEY_FALLING_EDGE};

QueueHandle_t key_queue = NULL;
QueueHandle_t key_inner_queue = NULL;

void task_key(void *parameter)
{
    key_runtime_state_t key_states[KEY_NUMS] = {0};

    uint32_t short_press_time = SHORT_PRESS_THRESHOLD_MS;
    uint32_t long_press_time = LONG_PRESS_THRESHOLD_MS;

    key_press_event_t *event = NULL;

    key_queue = xQueueCreate(20, sizeof(uint8_t));
    key_inner_queue = xQueueCreate(20, sizeof(key_press_event_t *));

    /* 创建队列失败，删除任务 */
    if (key_queue == NULL || key_inner_queue == NULL)
    {
        log_e("Failed to create key queues");
        vTaskDelete(NULL);
        return;
    }
    else
    {
        log_v("Key queues created successfully");
    }

    /* 主循环：处理按键事件 */
    for (;;)
    {
        /* 从内部队列接收按键事件，进行去抖、短按/长按判断，并把最终结果发送到外部队列 */
        if (xQueueReceive(key_inner_queue, &event, portMAX_DELAY) == pdPASS)
        {
            /* 参数校验 */
            if (event == NULL || event->key_id >= KEY_NUMS)
            {
                log_e("Invalid key event");
                continue;
            }

            key_runtime_state_t *state = &key_states[event->key_id];

            /* 1. 收到下降沿：表示按下 */
            if ((event->key_trigger_edge == KEY_FALLING_EDGE) &&
                (state->event_index == 0))
            {
                state->event_index = 1;
                state->first_trigger_time = event->key_trigger_time;
            }
            /* 2. 收到上升沿：表示释放 */
            else if ((event->key_trigger_edge == KEY_RAISING_EDGE) &&
                     (state->event_index == 1))
            {
                uint32_t press_duration = event->key_trigger_time - state->first_trigger_time;
                uint8_t final_key_value = KEY_NONE;

                /* 时间过短，认为无效 */
                if (press_duration < DEBOUNCE_TIME_MS)
                {
                    log_v("Invalid key press, too short: %lu ms", press_duration);
                }
                /* 短按 */
                else if (press_duration >= short_press_time && press_duration < long_press_time)
                {
                    switch (event->key_id)
                    {
                    case KEY_ID_1:
                        final_key_value = KEY1_PRESS;
                        break;
                    case KEY_ID_2:
                        final_key_value = KEY2_PRESS;
                        break;
                    case KEY_ID_3:
                        final_key_value = KEY3_PRESS;
                        break;
                    case KEY_ID_4:
                        final_key_value = KEY4_PRESS;
                        break;
                    default:
                        break;
                    }
                }
                /* 长按 */
                else if (press_duration >= long_press_time)
                {
                    switch (event->key_id)
                    {
                    case KEY_ID_1:
                        final_key_value = KEY1_LONG_PRESS;
                        break;
                    case KEY_ID_2:
                        final_key_value = KEY2_LONG_PRESS;
                        break;
                    case KEY_ID_3:
                        final_key_value = KEY3_LONG_PRESS;
                        break;
                    case KEY_ID_4:
                        final_key_value = KEY4_LONG_PRESS;
                        break;
                    default:
                        break;
                    }
                }

                if (final_key_value != KEY_NONE)
                {
                    if (xQueueSendToBack(key_queue, &final_key_value, 0) != pdPASS)
                    {
                        log_e("Failed to send final key value");
                    }
                }

                /* 状态机复位 */
                state->event_index = 0;
                state->first_trigger_time = 0;
            }
            else
            {
                /* 非法边沿顺序，复位状态机 */
                log_e("Invalid key edge sequence: key_id=%d edge=%d state=%d",
                      event->key_id,
                      event->key_trigger_edge,
                      state->event_index);

                state->event_index = 0;
                state->first_trigger_time = 0;
            }
        }
        else
        {
            log_e("Failed to receive key event from inner queue");
        }
    }
}

void key_gpio_exti_handler(uint16_t GPIO_Pin)
{
    static key_press_event_t key_events[KEY_NUMS][2] = {0};
    static uint32_t last_trigger_time[KEY_NUMS] = {0};

    const uint32_t debounce_interval = DEBOUNCE_TIME_MS;
    uint32_t current_tick = KEY_PRESS_GET_TIME_MS();

    key_press_event_t *p_key_press = NULL;
    key_id_t key_id;

    switch (GPIO_Pin)
    {
    case GPIO_PIN_0:
        key_id = KEY_ID_1;
        break;
    case GPIO_PIN_1:
        key_id = KEY_ID_2;
        break;
    case GPIO_PIN_2:
        key_id = KEY_ID_3;
        break;
    case GPIO_PIN_3:
        key_id = KEY_ID_4;
        break;
    default:
        return;
    }

    if ((current_tick - last_trigger_time[key_id]) < debounce_interval)
    {
        return;
    }
    last_trigger_time[key_id] = current_tick;

    if (key_inner_queue == NULL)
    {
        return;
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (irq_type[key_id] == KEY_FALLING_EDGE)
    {
        key_events[key_id][0].key_id = key_id;
        key_events[key_id][0].key_trigger_edge = KEY_FALLING_EDGE;
        key_events[key_id][0].key_trigger_time = current_tick;

        p_key_press = &key_events[key_id][0];

        if (xQueueSendFromISR(key_inner_queue, &p_key_press, &xHigherPriorityTaskWoken) != pdPASS)
        {
            log_e("Failed to send falling edge event");
        }

        irq_type[key_id] = KEY_RAISING_EDGE;
    }
    else
    {
        key_events[key_id][1].key_id = key_id;
        key_events[key_id][1].key_trigger_edge = KEY_RAISING_EDGE;
        key_events[key_id][1].key_trigger_time = current_tick;

        p_key_press = &key_events[key_id][1];

        if (xQueueSendFromISR(key_inner_queue, &p_key_press, &xHigherPriorityTaskWoken) != pdPASS)
        {
            log_e("Failed to send raising edge event");
        }

        irq_type[key_id] = KEY_FALLING_EDGE;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

uint8_t key_scan(uint8_t timeout_ms)
{
    uint8_t key_value = KEY_NONE;

    if (key_queue == NULL)
    {
        return KEY_NONE;
    }

    TickType_t ticks_to_wait = timeout_ms ? pdMS_TO_TICKS(timeout_ms) : 0;

    if (xQueueReceive(key_queue, &key_value, ticks_to_wait) == pdPASS)
    {
        return key_value;
    }

    return KEY_NONE;
}
