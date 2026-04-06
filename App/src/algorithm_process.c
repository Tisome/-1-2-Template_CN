#include "algorithm_process.h"
#include "algorithm_flow.h"
#include "app_config.h"

#define LOG_TAG "algo_proc"
#define LOG_LVL ELOG_LVL_ERROR

#include <stdbool.h>

#include "elog.h"

extern kalman_t kf;
extern ALARM_TYPE g_alarm;

bool algorithm_process_group(Pipe_Parameters_t *para,
                             Pipe_algo_state_t *state,
                             Pipe_algo_out_data_t *out,
                             double t1_ns,
                             double t2_ns,
                             double dt_ns)
{
    double sq = 0.0;
    bool is_bad = (t1_ns > T1_T2_LIMIT_NS) ||
                  (t2_ns > T1_T2_LIMIT_NS) ||
                  (dt_ns > DT_UP_LIMIT_NS) ||
                  (dt_ns < DT_LOW_LIMIT_NS);

    sq_window_update(state, is_bad);

    log_v("sq_window_update");

    sq = sq_get_percent(state);
    state->last_sq_value = sq;

    log_v("present SQ is %.3f", sq);

    if (is_bad)
    {
        log_v("this data is bad.");
        return false;
    }

    double flow_v_mps_raw = vel_calc_from_dt(para, t1_ns, t2_ns, dt_ns);

    log_v("raw flow speed is %.3f m/s", flow_v_mps_raw);

    double flow_v_avg = 0.0;
    if (!flow_window_add(state, &flow_v_mps_raw, &flow_v_avg))
    {
        log_v("don't out avg flow speed");
        return false;
    }

    log_v("avg flow speed is %.3f m/s", flow_v_avg);

    double flow_v_kalman = run_kalman_filter(&kf, flow_v_avg);

    log_v("after kalman flow speed is %.3f m/s", flow_v_kalman);

    double flow_v_comp = flow_drift_comp(para, state, flow_v_kalman, sq);

    log_v("after zero drift flow speed is %.3f m/s", flow_v_comp);

    state->last_flow_speed_mps = flow_v_comp;

    double flow_v_final = flow_limit(para, flow_v_comp);

    log_v("final flow speed is %.3f m/s", flow_v_final);

    update_flow_outputs(para, state, out, sq, flow_v_final);

    flow_alarm(para, flow_v_final);

    return true;
}
