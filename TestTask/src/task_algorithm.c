/*
 * 算法任务文件。
 * 本文件负责从接收队列取原始超声数据包，完成解包、时间量计算、流速/流量算法处理，
 * 再将结果写回全局输出和算法结果队列，供 GUI 与 Modbus 查询。
 */
#include "algorithm_packet.h"
#include "algorithm_process.h"
#include "app_config.h"
#include "freertos_resources.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <stdbool.h>

#define LOG_TAG "algo_task"
#define LOG_LVL ELOG_LVL_INFO
#include "elog.h"

#define ALGO_RX_TIMEOUT_MS         100U
#define ALGO_TIMEOUT_LOG_PERIOD_MS 1000U
#define ALGO_ERROR_LOG_PERIOD_MS   1000U
#define ALGO_OUTPUT_LOG_PERIOD_MS  APP_ALGO_OUTPUT_LOG_PERIOD_MS

/* 用于控制低频日志输出，避免高频任务把 RTT 日志刷爆。 */
static bool algo_should_log(TickType_t now_tick,
                            TickType_t *last_log_tick,
                            TickType_t period_tick)
{
    if (last_log_tick == NULL)
    {
        return false;
    }

    if ((*last_log_tick == 0U) || ((now_tick - *last_log_tick) >= period_tick))
    {
        *last_log_tick = now_tick;
        return true;
    }

    return false;
}

/* 按统一格式输出算法结果，便于 RTT / Ozone 中观察当前测量状态。 */
static void algo_log_output(const Pipe_algo_out_data_t *out, ALARM_TYPE alarm)
{
    if (out == NULL)
    {
        return;
    }

    log_i("algo out: v=%.3f %s, q=%.3f %s, total=%.3f %s, sq=%.1f, alarm=%d",
          out->flow_speed,
          speed_unit_to_str(out->flow_speed_unit),
          out->flow_rate_instant,
          rate_unit_to_str(out->flow_rate_unit),
          out->flow_rate_total,
          volume_unit_to_str(out->flow_total_unit),
          out->sq_value,
          alarm);
}

/*
 * 算法主任务。
 * 运行过程如下：
 * 1. 从 `xQueue_Rx_Index_Buf` 等待一帧原始数据；
 * 2. 解析为 idx/y 数据，并换算出 t1/t2/dt；
 * 3. 调用 `algorithm_process_group()` 完成整组算法处理；
 * 4. 将新结果写入 `xQueue_AlgoOut`，供 GUI 和 Modbus 读取。
 *
 * 本任务是“数据处理主链”的核心消费者，假数据任务或真实 SPI 接收任务都是它的上游。
 */
void task_algorithm(void *pvParameter)
{
    rufx_raw_packet_t raw = {0};
    bool has_last_seq = false;
    uint8_t last_seq = 0U;
    ALARM_TYPE last_logged_alarm = ALARM_OK;
    TickType_t last_timeout_log_tick = 0U;
    TickType_t last_error_log_tick = 0U;
    TickType_t last_output_log_tick = 0U;

    (void)pvParameter;

    log_i("task_algorithm start");

    while (1)
    {
        TickType_t now_tick = 0U;
        uint8_t cur_seq = 0U;
        rufx_packet_t pkt = {0};
        double t1_ns = 0.0;
        double t2_ns = 0.0;
        double dt_ns = 0.0;
        bool has_new_output = false;

        if ((xQueue_Rx_Index_Buf == NULL) || (xQueue_AlgoOut == NULL))
        {
            log_e("algorithm queues not ready");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        if (xQueueReceive(xQueue_Rx_Index_Buf, &raw, pdMS_TO_TICKS(ALGO_RX_TIMEOUT_MS)) != pdTRUE)
        {
            now_tick = xTaskGetTickCount();
            g_alarm = ALARM_OUT_OF_TIME;

            if (algo_should_log(now_tick,
                                &last_timeout_log_tick,
                                pdMS_TO_TICKS(ALGO_TIMEOUT_LOG_PERIOD_MS)))
            {
                log_e("rufx raw packet timeout");
            }
            continue;
        }

        g_alarm = ALARM_OK;
        rufx_unpack_packet(&raw, &pkt, &cur_seq);

        if (has_last_seq && (cur_seq == last_seq))
        {
            now_tick = xTaskGetTickCount();
            g_alarm = ALARM_REPEAT_PACKET;

            if (algo_should_log(now_tick,
                                &last_error_log_tick,
                                pdMS_TO_TICKS(ALGO_ERROR_LOG_PERIOD_MS)))
            {
                log_e("repeat packet seq=%u", cur_seq);
            }
            continue;
        }

        last_seq = cur_seq;
        has_last_seq = true;

        if (!rufx_calc_t1_t2_dt(&pkt, &t1_ns, &t2_ns, &dt_ns))
        {
            now_tick = xTaskGetTickCount();

            if (algo_should_log(now_tick,
                                &last_error_log_tick,
                                pdMS_TO_TICKS(ALGO_ERROR_LOG_PERIOD_MS)))
            {
                log_e("calc t1/t2/dt failed, seq=%u", cur_seq);
            }
            continue;
        }

        has_new_output = algorithm_process_group(&g_parameters,
                                                 &g_algo_state,
                                                 &g_algo_out,
                                                 t1_ns,
                                                 t2_ns,
                                                 dt_ns);
        if (!has_new_output)
        {
            continue;
        }

        (void)xQueueOverwrite(xQueue_AlgoOut, &g_algo_out);

        now_tick = xTaskGetTickCount();
        if (algo_should_log(now_tick,
                            &last_output_log_tick,
                            pdMS_TO_TICKS(ALGO_OUTPUT_LOG_PERIOD_MS)) ||
            (g_alarm != last_logged_alarm))
        {
            algo_log_output(&g_algo_out, g_alarm);
            last_logged_alarm = g_alarm;
        }
    }
}
