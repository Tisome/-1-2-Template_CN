#include "fake_data.h"
#include "freertos_resources.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <math.h>

#define LOG_TAG "fake_task"
#define LOG_LVL ELOG_LVL_INFO
#include "elog.h"

#define FAKE_DATA_CFG_REFRESH_MS 1000U
#define FAKE_DATA_LOG_PERIOD_MS  5000U

static double fake_pipe_area_m2(const Pipe_Parameters_t *para)
{
    double d_m = 0.0;
    double r_m = 0.0;

    if ((para == NULL) || (para->inner_diameter <= 0.0))
    {
        return 0.0;
    }

    d_m = para->inner_diameter * 1e-3;
    r_m = 0.5 * d_m;
    return M_PI * r_m * r_m;
}

static double convert_rate_to_m3ps(double rate, RateUnitType unit)
{
    switch (unit)
    {
    case RATE_UNIT_M3_P_H:
        return rate / 3600.0;

    case RATE_UNIT_M3_P_MIN:
        return rate / 60.0;

    case RATE_UNIT_L_P_H:
        return rate / (1000.0 * 3600.0);

    case RATE_UNIT_L_P_MIN:
        return rate / (1000.0 * 60.0);

    case RATE_UNIT_L_P_S:
        return rate / 1000.0;

    case RATE_UNIT_M3_P_S:
    default:
        return rate;
    }
}

static void fake_data_get_speed_range(const Pipe_Parameters_t *para,
                                      float *lower_speed_mps,
                                      float *upper_speed_mps)
{
    double area_m2 = 0.0;
    double min_speed = 0.0;
    double max_speed = 0.0;
    double speed_low = 0.0;
    double speed_high = 0.0;
    double alarm_low_speed = 0.0;
    double alarm_high_speed = 0.0;

    if ((para == NULL) || (lower_speed_mps == NULL) || (upper_speed_mps == NULL))
    {
        return;
    }

    speed_low = para->lower_speed_range * 1.5;
    speed_high = para->upper_speed_range * 0.3;
    min_speed = para->lower_speed_range * 1.2;
    max_speed = para->upper_speed_range * 0.9;

    if (min_speed < 0.10)
    {
        min_speed = 0.10;
    }

    if (max_speed <= min_speed)
    {
        max_speed = min_speed + 0.50;
    }

    if (speed_low < min_speed)
    {
        speed_low = min_speed;
    }

    if (speed_high > max_speed)
    {
        speed_high = max_speed;
    }

    if (speed_high <= speed_low)
    {
        speed_high = speed_low + 0.50;
    }

    area_m2 = fake_pipe_area_m2(para);
    if ((area_m2 > 0.0) &&
        (para->alarm_upper_rate_range > para->alarm_lower_rate_range) &&
        (para->alarm_upper_rate_range > 0.0))
    {
        alarm_low_speed = fabs(convert_rate_to_m3ps(para->alarm_lower_rate_range,
                                                    para->rate_unit_type)) /
                          area_m2;
        alarm_high_speed = fabs(convert_rate_to_m3ps(para->alarm_upper_rate_range,
                                                     para->rate_unit_type)) /
                           area_m2;

        if (alarm_low_speed > 0.0)
        {
            speed_low = alarm_low_speed * 0.7;
        }

        if (alarm_high_speed > 0.0)
        {
            speed_high = alarm_high_speed * 1.3;
        }
    }

    if (speed_low < min_speed)
    {
        speed_low = min_speed;
    }

    if (speed_low >= max_speed)
    {
        speed_low = min_speed;
    }

    if (speed_high > max_speed)
    {
        speed_high = max_speed;
    }

    if (speed_high <= speed_low)
    {
        speed_high = speed_low + 0.50;
    }

    if (speed_high > max_speed)
    {
        speed_high = max_speed;
    }

    if (speed_high <= speed_low)
    {
        speed_low = min_speed;
        speed_high = max_speed;
    }

    *lower_speed_mps = (float)speed_low;
    *upper_speed_mps = (float)speed_high;
}

static void fake_data_refresh_cfg(const Pipe_Parameters_t *para,
                                  fake_data_cfg_t *cfg)
{
    float lower_speed_mps = 0.0f;
    float upper_speed_mps = 0.0f;

    if ((para == NULL) || (cfg == NULL))
    {
        return;
    }

    fake_data_get_speed_range(para, &lower_speed_mps, &upper_speed_mps);

#if FAKE_DATA_MODE == FAKE_DATA_MODE_SPEED
    fake_data_set_cfg(lower_speed_mps, upper_speed_mps, 12.0f);
#else
    {
        double area_m2 = fake_pipe_area_m2(para);
        double lower_m3ps = (double)lower_speed_mps * area_m2;
        double upper_m3ps = (double)upper_speed_mps * area_m2;

        fake_data_set_cfg((float)convert_rate_from_m3ps(lower_m3ps, RATE_UNIT_L_P_MIN),
                          (float)convert_rate_from_m3ps(upper_m3ps, RATE_UNIT_L_P_MIN),
                          12.0f);
    }
#endif

    fake_data_get_cfg(cfg);
}

static void fake_data_log_cfg(const fake_data_cfg_t *cfg,
                              const Pipe_Parameters_t *para,
                              float time_s)
{
    float target_speed_mps = 0.0f;

    if ((cfg == NULL) || (para == NULL))
    {
        return;
    }

    target_speed_mps = fake_data_get_target_speed_mps(time_s, para);

#if FAKE_DATA_MODE == FAKE_DATA_MODE_SPEED
    log_i("fake cfg: %.2f~%.2f m/s, period=%.1f s, target=%.2f m/s",
          cfg->lower,
          cfg->upper,
          cfg->period_s,
          target_speed_mps);
#else
    log_i("fake cfg: %.2f~%.2f L/min, period=%.1f s, target=%.2f m/s",
          cfg->lower,
          cfg->upper,
          cfg->period_s,
          target_speed_mps);
#endif
}

void task_fake_data(void *parameter)
{
    TickType_t last_wake_tick = 0U;
    TickType_t last_cfg_refresh_tick = 0U;
    TickType_t last_log_tick = 0U;
    fake_data_cfg_t cfg = {0};
    uint32_t sample_index = 0U;
    uint8_t seq = 0U;

    (void)parameter;

    log_i("task_fake_data start");

    last_wake_tick = xTaskGetTickCount();
    last_cfg_refresh_tick = last_wake_tick;
    last_log_tick = last_wake_tick;

    fake_data_refresh_cfg(&g_parameters, &cfg);
    fake_data_log_cfg(&cfg, &g_parameters, 0.0f);

    for (;;)
    {
        TickType_t now_tick = xTaskGetTickCount();
        float time_s = (float)sample_index * ((float)GROUP_PERIOD_MS / 1000.0f);
        rufx_raw_packet_t raw = {0};

        if (xQueue_Rx_Index_Buf == NULL)
        {
            log_e("xQueue_Rx_Index_Buf is NULL");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        if (fake_data_consume_cfg_refresh_request() ||
            ((now_tick - last_cfg_refresh_tick) >= pdMS_TO_TICKS(FAKE_DATA_CFG_REFRESH_MS)))
        {
            fake_data_refresh_cfg(&g_parameters, &cfg);
            last_cfg_refresh_tick = now_tick;
        }

        fake_data_make_packet(&raw, time_s, &g_parameters);
        raw.seq = seq++;

        (void)xQueueOverwrite(xQueue_Rx_Index_Buf, &raw);

        if ((now_tick - last_log_tick) >= pdMS_TO_TICKS(FAKE_DATA_LOG_PERIOD_MS))
        {
            fake_data_log_cfg(&cfg, &g_parameters, time_s);
            last_log_tick = now_tick;
        }

        sample_index++;
        vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(GROUP_PERIOD_MS));
    }
}
