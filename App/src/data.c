#include "data.h"
#include "algorithm_flow.h"
#include "at24cxx_handler.h"
#include "fake_data.h"
#include "freertos_resources.h"
#include "modbus_frame_process.h"
#include "elog.h"

#include <stdbool.h>
#include <math.h>
#include <string.h>

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
        .flow_rate_unit = RATE_UNIT_M3_P_H,
        .flow_total_unit = VOLUME_UNIT_M3};

ALARM_TYPE g_alarm = ALARM_OK;

static const Pipe_Parameters_t *get_default_pipe_parameters_ptr(void)
{
    static const Pipe_Parameters_t s_default_pipe_parameters =
        {
            .inner_diameter = 20.0,
            .wall_thick = 1.0,
            .cos_value = 0.913545,
            .sin_value = 0.406738,

            .lower_speed_range = 0.05,
            .upper_speed_range = 20.0,

            .alarm_lower_rate_range = 2.0,
            .alarm_upper_rate_range = 20.0,

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
            .rate_unit_type = RATE_UNIT_M3_P_H};

    return &s_default_pipe_parameters;
}

static Pipe_Parameters_t make_default_pipe_parameters(void)
{
    return *get_default_pipe_parameters_ptr();
}

static kalman_t make_default_kalman_state(void)
{
    kalman_t default_kalman =
        {
            .x = 0.0,
            .p = 15.0,
            .q = 0.005,
            .r = 0.1,
            .k = 0.0};

    return default_kalman;
}

static Pipe_algo_state_t make_default_algo_state(void)
{
    Pipe_algo_state_t default_state = {0};
    return default_state;
}

static Pipe_algo_out_data_t make_default_algo_out(const Pipe_Parameters_t *para)
{
    Pipe_algo_out_data_t default_out = {0};
    RateUnitType rate_unit = RATE_UNIT_L_P_MIN;
    SpeedUnitType speed_unit = SPEED_UNIT_M_P_S;

    if (para != NULL)
    {
        rate_unit = para->rate_unit_type;
        speed_unit = para->speed_unit_type;
    }

    default_out.flow_speed = 0.0;
    default_out.flow_rate_instant = 0.0;
    default_out.flow_rate_total = 0.0;
    default_out.sq_value = 0.0;
    default_out.flow_speed_unit = speed_unit;
    default_out.flow_rate_unit = rate_unit;
    default_out.flow_total_unit = volume_unit_from_rate_unit(rate_unit);

    return default_out;
}

static bool is_valid_speed_unit(uint32_t value)
{
    return value <= (uint32_t)SPEED_UNIT_MM_P_S;
}

static bool is_valid_rate_unit(uint32_t value)
{
    return value <= (uint32_t)RATE_UNIT_L_P_S;
}

static bool is_valid_pipe_type(uint32_t value)
{
    return value <= (uint32_t)PIPE_ALLOY;
}

static bool parameter_double_equal(double lhs, double rhs)
{
    return fabs(lhs - rhs) < 1e-9;
}

static bool parameter_change_requires_measurement_reset(const Pipe_Parameters_t *old_para,
                                                        const Pipe_Parameters_t *new_para)
{
    if ((old_para == NULL) || (new_para == NULL))
    {
        return false;
    }

    if (!parameter_double_equal(old_para->inner_diameter, new_para->inner_diameter) ||
        !parameter_double_equal(old_para->wall_thick, new_para->wall_thick) ||
        !parameter_double_equal(old_para->cos_value, new_para->cos_value) ||
        !parameter_double_equal(old_para->sin_value, new_para->sin_value) ||
        !parameter_double_equal(old_para->lower_speed_range, new_para->lower_speed_range) ||
        !parameter_double_equal(old_para->upper_speed_range, new_para->upper_speed_range) ||
        !parameter_double_equal(old_para->alarm_lower_rate_range, new_para->alarm_lower_rate_range) ||
        !parameter_double_equal(old_para->alarm_upper_rate_range, new_para->alarm_upper_rate_range) ||
        !parameter_double_equal(old_para->zero_offset_speed, new_para->zero_offset_speed) ||
        !parameter_double_equal(old_para->zero_learn_flow_speed, new_para->zero_learn_flow_speed) ||
        !parameter_double_equal(old_para->zero_learn_alpha, new_para->zero_learn_alpha) ||
        !parameter_double_equal(old_para->zero_learn_offset_max, new_para->zero_learn_offset_max) ||
        !parameter_double_equal(old_para->zero_learn_sq_min, new_para->zero_learn_sq_min) ||
        !parameter_double_equal(old_para->te_ns, new_para->te_ns) ||
        (old_para->pipe_type != new_para->pipe_type) ||
        (old_para->speed_unit_type != new_para->speed_unit_type) ||
        (old_para->rate_unit_type != new_para->rate_unit_type))
    {
        return true;
    }

    return false;
}

static bool parameter_configuration_equal(const Pipe_Parameters_t *lhs,
                                          const Pipe_Parameters_t *rhs)
{
    if ((lhs == NULL) || (rhs == NULL))
    {
        return false;
    }

    return parameter_double_equal(lhs->inner_diameter, rhs->inner_diameter) &&
           parameter_double_equal(lhs->wall_thick, rhs->wall_thick) &&
           parameter_double_equal(lhs->cos_value, rhs->cos_value) &&
           parameter_double_equal(lhs->sin_value, rhs->sin_value) &&
           parameter_double_equal(lhs->lower_speed_range, rhs->lower_speed_range) &&
           parameter_double_equal(lhs->upper_speed_range, rhs->upper_speed_range) &&
           parameter_double_equal(lhs->alarm_lower_rate_range, rhs->alarm_lower_rate_range) &&
           parameter_double_equal(lhs->alarm_upper_rate_range, rhs->alarm_upper_rate_range) &&
           parameter_double_equal(lhs->zero_offset_speed, rhs->zero_offset_speed) &&
           parameter_double_equal(lhs->zero_learn_flow_speed, rhs->zero_learn_flow_speed) &&
           parameter_double_equal(lhs->zero_learn_alpha, rhs->zero_learn_alpha) &&
           parameter_double_equal(lhs->zero_learn_offset_max, rhs->zero_learn_offset_max) &&
           parameter_double_equal(lhs->zero_learn_sq_min, rhs->zero_learn_sq_min) &&
           parameter_double_equal(lhs->te_ns, rhs->te_ns) &&
           (lhs->output_mode == rhs->output_mode) &&
           (lhs->display_sensitivity == rhs->display_sensitivity) &&
           (lhs->zero_stable_threshold == rhs->zero_stable_threshold) &&
#if USE_MODBUS
           (lhs->modbus_addr == rhs->modbus_addr) &&
#endif
           (lhs->pipe_type == rhs->pipe_type) &&
           (lhs->speed_unit_type == rhs->speed_unit_type) &&
           (lhs->rate_unit_type == rhs->rate_unit_type);
}

static void parameter_sync_external_state(void)
{
    update_holding_registers_from_parameters();
    update_input_registers();
    fake_data_request_cfg_refresh();
}

static parameter_apply_status_t parameter_status_from_eeprom(e2prom_status_t status)
{
    switch (status)
    {
    case E2PROM_OK:
        return PARAMETER_APPLY_OK;

    case E2PROM_BUSY:
        return PARAMETER_APPLY_BUSY;

    default:
        return PARAMETER_APPLY_SAVE_FAILED;
    }
}

bool parameter_storage_is_persistent(void)
{
#if USE_E2PROM
    return true;
#else
    return false;
#endif
}

static uint32_t parameter_saved_flag_for_board(void)
{
    return parameter_storage_is_persistent() ? 1U : 0U;
}

static parameter_apply_status_t parameter_try_save_current_state(Pipe_Parameters_t *para)
{
    e2prom_status_t save_status = E2PROM_OK;

    if (para == NULL)
    {
        return PARAMETER_APPLY_INVALID;
    }

    if (!parameter_storage_is_persistent())
    {
        para->is_saved = 0U;
        return PARAMETER_APPLY_OK;
    }

    para->is_saved = 1U;
    save_status = SaveParameters(para);
    if (save_status != E2PROM_OK)
    {
        para->is_saved = 0U;
    }

    return parameter_status_from_eeprom(save_status);
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
        log_i("parameter init: load eeprom failed, use default and save");
        return;
    }

    if (g_parameters.is_saved != 1U)
    {
        g_parameters = default_pipe_parameters;
        g_parameters.is_saved = 1U;
        (void)SaveParameters(&g_parameters);
        log_i("parameter init: eeprom empty, use default and save");
    }
    else
    {
        log_i("parameter init: load from eeprom");
    }
#else
    g_parameters = default_pipe_parameters;
    log_i("parameter init: use default, eeprom disabled on this board");
#endif

    kf = make_default_kalman_state();
    g_algo_state = make_default_algo_state();
    g_algo_out = make_default_algo_out(&g_parameters);
    g_alarm = ALARM_OK;
    parameter_sync_external_state();
}

void parameter_reset_to_default(void)
{
    g_parameters = make_default_pipe_parameters();
}

bool parameter_validate(const Pipe_Parameters_t *para)
{
    if (para == NULL)
    {
        return false;
    }

    if ((para->inner_diameter <= 0.0) || (para->inner_diameter > 10000.0))
    {
        return false;
    }

    if ((para->wall_thick <= 0.0) || (para->wall_thick >= para->inner_diameter))
    {
        return false;
    }

    if ((para->cos_value <= 0.0) || (para->cos_value > 1.0))
    {
        return false;
    }

    if ((para->sin_value <= 0.0) || (para->sin_value > 1.0))
    {
        return false;
    }

    if ((para->lower_speed_range < 0.0) ||
        (para->upper_speed_range <= 0.0) ||
        (para->lower_speed_range >= para->upper_speed_range))
    {
        return false;
    }

    if ((para->alarm_lower_rate_range < 0.0) ||
        (para->alarm_upper_rate_range < para->alarm_lower_rate_range))
    {
        return false;
    }

    if ((fabs(para->zero_offset_speed) > para->zero_learn_offset_max) ||
        (para->zero_learn_flow_speed < 0.0) ||
        (para->zero_learn_alpha <= 0.0) ||
        (para->zero_learn_alpha > 1.0) ||
        (para->zero_learn_offset_max < 0.0))
    {
        return false;
    }

    if ((para->zero_learn_sq_min < 0.0) || (para->zero_learn_sq_min > 100.0))
    {
        return false;
    }

    if (para->te_ns < 0.0)
    {
        return false;
    }

    if (para->zero_stable_threshold == 0U)
    {
        return false;
    }

#if USE_MODBUS
    if ((para->modbus_addr == 0U) || (para->modbus_addr > 247U))
    {
        return false;
    }
#endif

    if (!is_valid_pipe_type((uint32_t)para->pipe_type) ||
        !is_valid_speed_unit((uint32_t)para->speed_unit_type) ||
        !is_valid_rate_unit((uint32_t)para->rate_unit_type))
    {
        return false;
    }

    return true;
}

void parameter_reset_measurement_state(void)
{
    g_algo_state = make_default_algo_state();
    g_algo_out = make_default_algo_out(&g_parameters);
    g_alarm = ALARM_OK;
    kf = make_default_kalman_state();

    if (xQueue_AlgoOut != NULL)
    {
        (void)xQueueOverwrite(xQueue_AlgoOut, &g_algo_out);
    }

    update_input_registers();
}

bool parameter_get_double(parameter_field_id_t field_id, double *value)
{
    if (value == NULL)
    {
        return false;
    }

    switch (field_id)
    {
    case PARAMETER_FIELD_INNER_DIAMETER:
        *value = g_parameters.inner_diameter;
        return true;

    case PARAMETER_FIELD_WALL_THICK:
        *value = g_parameters.wall_thick;
        return true;

    case PARAMETER_FIELD_ALARM_LOWER_RATE_RANGE:
        *value = g_parameters.alarm_lower_rate_range;
        return true;

    case PARAMETER_FIELD_ALARM_UPPER_RATE_RANGE:
        *value = g_parameters.alarm_upper_rate_range;
        return true;

    case PARAMETER_FIELD_ZERO_OFFSET_SPEED:
        *value = g_parameters.zero_offset_speed;
        return true;

    case PARAMETER_FIELD_ZERO_LEARN_FLOW_SPEED:
        *value = g_parameters.zero_learn_flow_speed;
        return true;

    case PARAMETER_FIELD_ZERO_LEARN_ALPHA:
        *value = g_parameters.zero_learn_alpha;
        return true;

    case PARAMETER_FIELD_ZERO_LEARN_OFFSET_MAX:
        *value = g_parameters.zero_learn_offset_max;
        return true;

    case PARAMETER_FIELD_ZERO_LEARN_SQ_MIN:
        *value = g_parameters.zero_learn_sq_min;
        return true;

    default:
        return false;
    }
}

bool parameter_get_u32(parameter_field_id_t field_id, uint32_t *value)
{
    if (value == NULL)
    {
        return false;
    }

    switch (field_id)
    {
    case PARAMETER_FIELD_OUTPUT_MODE:
        *value = g_parameters.output_mode;
        return true;

    case PARAMETER_FIELD_DISPLAY_SENSITIVITY:
        *value = g_parameters.display_sensitivity;
        return true;

    case PARAMETER_FIELD_ZERO_STABLE_THRESHOLD:
        *value = g_parameters.zero_stable_threshold;
        return true;

    case PARAMETER_FIELD_MODBUS_ADDR:
        *value = (uint32_t)g_parameters.modbus_addr;
        return true;

    case PARAMETER_FIELD_PIPE_TYPE:
        *value = (uint32_t)g_parameters.pipe_type;
        return true;

    case PARAMETER_FIELD_SPEED_UNIT_TYPE:
        *value = (uint32_t)g_parameters.speed_unit_type;
        return true;

    case PARAMETER_FIELD_RATE_UNIT_TYPE:
        *value = (uint32_t)g_parameters.rate_unit_type;
        return true;

    default:
        return false;
    }
}

parameter_apply_status_t parameter_commit(const Pipe_Parameters_t *candidate)
{
    Pipe_Parameters_t old_parameters = g_parameters;
    Pipe_Parameters_t new_parameters;
    bool need_measurement_reset = false;
    parameter_apply_status_t apply_status = PARAMETER_APPLY_OK;

    if (candidate == NULL)
    {
        return PARAMETER_APPLY_INVALID;
    }

    new_parameters = *candidate;
    new_parameters.is_saved = parameter_saved_flag_for_board();

    if (!parameter_validate(&new_parameters))
    {
        return PARAMETER_APPLY_INVALID;
    }

    need_measurement_reset = parameter_change_requires_measurement_reset(&old_parameters,
                                                                        &new_parameters);

    if (parameter_configuration_equal(&old_parameters, &new_parameters) &&
        (old_parameters.is_saved == new_parameters.is_saved))
    {
        g_parameters.is_saved = new_parameters.is_saved;
        parameter_sync_external_state();
        return PARAMETER_APPLY_OK;
    }

    g_parameters = new_parameters;

    apply_status = parameter_try_save_current_state(&g_parameters);
    if (apply_status != PARAMETER_APPLY_OK)
    {
        g_parameters = old_parameters;
        parameter_sync_external_state();
        return apply_status;
    }

    if (need_measurement_reset)
    {
        parameter_reset_measurement_state();
    }

    parameter_sync_external_state();
    return PARAMETER_APPLY_OK;
}

parameter_apply_status_t parameter_save_current(void)
{
    parameter_apply_status_t apply_status = PARAMETER_APPLY_OK;

    if (!parameter_validate(&g_parameters))
    {
        return PARAMETER_APPLY_INVALID;
    }

    apply_status = parameter_try_save_current_state(&g_parameters);
    parameter_sync_external_state();
    return apply_status;
}

parameter_apply_status_t parameter_set_double(parameter_field_id_t field_id, double value)
{
    Pipe_Parameters_t candidate = g_parameters;

    switch (field_id)
    {
    case PARAMETER_FIELD_INNER_DIAMETER:
        candidate.inner_diameter = value;
        break;

    case PARAMETER_FIELD_WALL_THICK:
        candidate.wall_thick = value;
        break;

    case PARAMETER_FIELD_ALARM_LOWER_RATE_RANGE:
        candidate.alarm_lower_rate_range = value;
        break;

    case PARAMETER_FIELD_ALARM_UPPER_RATE_RANGE:
        candidate.alarm_upper_rate_range = value;
        break;

    case PARAMETER_FIELD_ZERO_OFFSET_SPEED:
        candidate.zero_offset_speed = value;
        break;

    case PARAMETER_FIELD_ZERO_LEARN_FLOW_SPEED:
        candidate.zero_learn_flow_speed = value;
        break;

    case PARAMETER_FIELD_ZERO_LEARN_ALPHA:
        candidate.zero_learn_alpha = value;
        break;

    case PARAMETER_FIELD_ZERO_LEARN_OFFSET_MAX:
        candidate.zero_learn_offset_max = value;
        break;

    case PARAMETER_FIELD_ZERO_LEARN_SQ_MIN:
        candidate.zero_learn_sq_min = value;
        break;

    default:
        return PARAMETER_APPLY_UNSUPPORTED;
    }

    return parameter_commit(&candidate);
}

parameter_apply_status_t parameter_set_u32(parameter_field_id_t field_id, uint32_t value)
{
    Pipe_Parameters_t candidate = g_parameters;

    switch (field_id)
    {
    case PARAMETER_FIELD_OUTPUT_MODE:
        candidate.output_mode = value;
        break;

    case PARAMETER_FIELD_DISPLAY_SENSITIVITY:
        candidate.display_sensitivity = value;
        break;

    case PARAMETER_FIELD_ZERO_STABLE_THRESHOLD:
        candidate.zero_stable_threshold = value;
        break;

    case PARAMETER_FIELD_MODBUS_ADDR:
        candidate.modbus_addr = (uint8_t)value;
        break;

    case PARAMETER_FIELD_PIPE_TYPE:
        candidate.pipe_type = (PipeType)value;
        break;

    case PARAMETER_FIELD_SPEED_UNIT_TYPE:
        candidate.speed_unit_type = (SpeedUnitType)value;
        break;

    case PARAMETER_FIELD_RATE_UNIT_TYPE:
        candidate.rate_unit_type = (RateUnitType)value;
        break;

    default:
        return PARAMETER_APPLY_UNSUPPORTED;
    }

    return parameter_commit(&candidate);
}

parameter_apply_status_t parameter_execute_action(parameter_action_t action)
{
    switch (action)
    {
    case PARAMETER_ACTION_CLEAR_TOTALIZER:
        g_algo_state.q_total_m3 = 0.0;
        g_algo_out.flow_rate_total = 0.0;
        update_input_registers();
        return PARAMETER_APPLY_OK;

    case PARAMETER_ACTION_ZERO_LEARN_START:
        if (!zero_learn_manual_start(&g_parameters, &g_algo_state))
        {
            return PARAMETER_APPLY_INVALID;
        }

        return parameter_save_current();

    case PARAMETER_ACTION_LOAD_DEFAULTS:
        return parameter_commit(get_default_pipe_parameters_ptr());

    case PARAMETER_ACTION_CLEAR_ALARM:
        g_alarm = ALARM_OK;
        update_input_registers();
        return PARAMETER_APPLY_OK;

    default:
        return PARAMETER_APPLY_UNSUPPORTED;
    }
}

/* ========================= EEPROM 参数接口 ========================= */

e2prom_status_t SaveParameters(Pipe_Parameters_t *para)
{
    if (para == NULL)
    {
        return E2PROM_ERROR;
    }

#if USE_E2PROM
    /* Parameter storage currently uses the simple sync EEPROM API. */
    return eeprom_write_sync(E2PROM_PIPE_PARA_START_ADDR,
                             (const uint8_t *)para,
                             sizeof(Pipe_Parameters_t));
#else
    (void)para;
    return E2PROM_ERROR_RESOURCE;
#endif
}

e2prom_status_t LoadParameters(Pipe_Parameters_t *para)
{
    if (para == NULL)
    {
        return E2PROM_ERROR;
    }

#if USE_E2PROM
    return eeprom_read_sync(E2PROM_PIPE_PARA_START_ADDR,
                            (uint8_t *)para,
                            sizeof(Pipe_Parameters_t));
#else
    (void)para;
    return E2PROM_ERROR_RESOURCE;
#endif
}
