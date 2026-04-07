/*
 * 单组算法处理文件。
 * 本文件把已经计算好的 t1 / t2 / dt 继续转换为流速、流量和报警状态，
 * 是“时延量 -> 流量结果”的核心封装层。
 */
#include "algorithm_process.h"
#include "algorithm_flow.h"
#include "app_config.h"

#define LOG_TAG "algo_proc"
#define LOG_LVL ELOG_LVL_ERROR

#include <stdbool.h>

#include "elog.h"

extern kalman_t kf;
extern ALARM_TYPE g_alarm;

/*
 * 处理一组算法输入并尝试生成新的输出结果。
 * 运行流程为：
 * 1. 先按 t1 / t2 / dt 判断本组数据是否明显异常；
 * 2. 更新 SQ 统计窗口；
 * 3. 计算原始流速；
 * 4. 经过滑动窗口平均、卡尔曼滤波、零漂补偿和上下限裁剪；
 * 5. 更新最终输出和报警状态。
 *
 * 只有在“窗口条件满足且本轮产生了新平均值”时才返回 true。
 */
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
