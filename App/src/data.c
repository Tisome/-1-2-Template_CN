#include "data.h"
#include "at24cxx_handler.h"
#include "elog.h"

#include <stdbool.h>

/* ========================= 全局数据定义 ========================= */

Pipe_Parameters_t g_parameters = {0};

kalman_t kf =
    {
        .x = 0.0,
        .p = 15.0,
        .q = 0.005,
        .r = 0.1,
        .k = 0.0};

Pipe_algo_state_t g_algo_state =
    {
        .zero_stable = 0,

        .bad_flags = {0},
        .sq_idx = 0,
        .sq_count = 0,
        .sq_bad_count = 0,

        .window_buf = {0},
        .window_idx = 0,
        .step_cnt = 0,
        .window_full = false,

        .q_total_m3 = 0.0,
        .last_flow_speed_mps = 0.0,
        .last_sq_value = 0.0};

Pipe_algo_out_data_t g_algo_out =
    {
        .flow_speed = 0.0,
        .flow_rate_instant = 0.0,
        .flow_rate_total = 0.0,
        .sq_value = 0.0,
        .flow_speed_unit = SPEED_UNIT_M_P_S,
        .flow_rate_unit = RATE_UNIT_M3_P_S,
        .flow_total_unit = VOLUME_UNIT_M3};

ALARM_TYPE g_alarm = ALARM_OK;

static Pipe_Parameters_t make_default_pipe_parameters(void)
{
    Pipe_Parameters_t default_pipe_parameters =
        {
            .inner_diameter = 20.0,
            .wall_thick = 1.0,
            .cos_value = 0.913545,
            .sin_value = 0.406738,

            .lower_speed_range = 0.05,
            .upper_speed_range = 20.0,

            .alarm_lower_rate_range = 10.0,
            .alarm_upper_rate_range = 40.0,

            .zero_offset_speed = 0.0,
            .zero_learn_flow_speed = 0.08,
            .zero_learn_alpha = 0.005,
            .zero_learn_offset_max = 0.2,
            .zero_learn_sq_min = 95.0,

            .te_ns = 12000.0,

            .is_saved = 0U,
            .output_mode = 0U,
            .display_sensitivity = 5U,
            .zero_stable_threshold = 150U,

#if USE_MODBUS
            .modbus_addr = 0x01U,
#endif

            .pipe_type = PIPE_PVC,
            .speed_unit_type = SPEED_UNIT_M_P_S,
            .rate_unit_type = RATE_UNIT_M3_P_S};

    return default_pipe_parameters;
}

double convert_speed_from_mps(double speed_mps, SpeedUnitType unit)
{
    switch (unit)
    {
    case SPEED_UNIT_CM_P_S:
        return speed_mps * 100.0;

    case SPEED_UNIT_MM_P_S:
        return speed_mps * 1000.0;

    case SPEED_UNIT_M_P_S:
    default:
        return speed_mps;
    }
}

double convert_rate_from_m3ps(double rate_m3ps, RateUnitType unit)
{
    switch (unit)
    {
    case RATE_UNIT_M3_P_H:
        return rate_m3ps * 3600.0;

    case RATE_UNIT_M3_P_MIN:
        return rate_m3ps * 60.0;

    case RATE_UNIT_L_P_H:
        return rate_m3ps * 1000.0 * 3600.0;

    case RATE_UNIT_L_P_MIN:
        return rate_m3ps * 1000.0 * 60.0;

    case RATE_UNIT_L_P_S:
        return rate_m3ps * 1000.0;

    case RATE_UNIT_M3_P_S:
    default:
        return rate_m3ps;
    }
}

double convert_volume_from_m3(double volume_m3, VolumeUnitType unit)
{
    switch (unit)
    {
    case VOLUME_UNIT_L:
        return volume_m3 * 1000.0;

    case VOLUME_UNIT_M3:
    default:
        return volume_m3;
    }
}

VolumeUnitType volume_unit_from_rate_unit(RateUnitType rate_unit)
{
    switch (rate_unit)
    {
    case RATE_UNIT_L_P_H:
    case RATE_UNIT_L_P_MIN:
    case RATE_UNIT_L_P_S:
        return VOLUME_UNIT_L;

    case RATE_UNIT_M3_P_H:
    case RATE_UNIT_M3_P_MIN:
    case RATE_UNIT_M3_P_S:
    default:
        return VOLUME_UNIT_M3;
    }
}

const char *speed_unit_to_str(SpeedUnitType unit)
{
    switch (unit)
    {
    case SPEED_UNIT_CM_P_S:
        return "cm/s";

    case SPEED_UNIT_MM_P_S:
        return "mm/s";

    case SPEED_UNIT_M_P_S:
    default:
        return "m/s";
    }
}

const char *rate_unit_to_str(RateUnitType unit)
{
    switch (unit)
    {
    case RATE_UNIT_M3_P_H:
        return "m3/h";

    case RATE_UNIT_M3_P_MIN:
        return "m3/min";

    case RATE_UNIT_L_P_H:
        return "L/h";

    case RATE_UNIT_L_P_MIN:
        return "L/min";

    case RATE_UNIT_L_P_S:
        return "L/s";

    case RATE_UNIT_M3_P_S:
    default:
        return "m3/s";
    }
}

const char *volume_unit_to_str(VolumeUnitType unit)
{
    switch (unit)
    {
    case VOLUME_UNIT_L:
        return "L";

    case VOLUME_UNIT_M3:
    default:
        return "m3";
    }
}

/* ========================= 参数初始化 ========================= */

void parameter_init(void)
{
    Pipe_Parameters_t default_pipe_parameters = make_default_pipe_parameters();

#if USE_E2PROM
    if (LoadParameters(&g_parameters) != E2PROM_OK)
    {
        g_parameters = default_pipe_parameters;
        g_parameters.is_saved = 1U;
        (void)SaveParameters(&g_parameters);
        return;
    }

    if (g_parameters.is_saved != 1U)
    {
        g_parameters = default_pipe_parameters;
        g_parameters.is_saved = 1U;
        (void)SaveParameters(&g_parameters);
    }
#else
    g_parameters = default_pipe_parameters;
#endif
    log_v("parameter loaded");
}

void parameter_reset_to_default(void)
{
    g_parameters = make_default_pipe_parameters();
}

/* ========================= EEPROM 参数接口 ========================= */

e2prom_status_t SaveParameters(Pipe_Parameters_t *para)
{
    if (para == NULL)
    {
        return E2PROM_ERROR;
    }

    /* Parameter storage currently uses the simple sync EEPROM API. */
    return eeprom_write_sync(E2PROM_PIPE_PARA_START_ADDR,
                             (const uint8_t *)para,
                             sizeof(Pipe_Parameters_t));
}

e2prom_status_t LoadParameters(Pipe_Parameters_t *para)
{
    if (para == NULL)
    {
        return E2PROM_ERROR;
    }

    return eeprom_read_sync(E2PROM_PIPE_PARA_START_ADDR,
                            (uint8_t *)para,
                            sizeof(Pipe_Parameters_t));
}
