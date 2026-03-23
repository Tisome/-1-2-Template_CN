#include "algorithm_flow.h"
#include <math.h>
#include <string.h>

#include "elog.h"
#define LOG_TAG "algo_flow_out"
#define LOG_LVL ELOG_LVL_VERBOSE

// 更新 3 秒窗口坏数据统计（环形缓冲）
void sq_window_update(Pipe_algo_state_t *s, bool is_bad)
{
    if (s->sq_count < SQ_WINDOW_GROUPS)
    {
        s->bad_flags[s->sq_idx] = is_bad ? 1U : 0U;
        s->sq_bad_count += is_bad ? 1U : 0U;
        s->sq_count++;
        s->sq_idx = (uint16_t)((s->sq_idx + 1U) % SQ_WINDOW_GROUPS);
        return;
    }

    // 窗口已满：覆盖最旧数据
    s->sq_bad_count -= s->bad_flags[s->sq_idx];
    s->bad_flags[s->sq_idx] = is_bad ? 1U : 0U;
    s->sq_bad_count += is_bad ? 1U : 0U;
    s->sq_idx = (uint16_t)((s->sq_idx + 1U) % SQ_WINDOW_GROUPS);
}

// 获取 SQ（好数据百分比）
double sq_get_percent(const Pipe_algo_state_t *s)
{
    uint16_t denom = (s->sq_count == 0U) ? 1U : s->sq_count;
    double bad = s->sq_bad_count;
    double total = (double)denom;
    return 100.0 * (1.0 - (bad / total));
}

double calc_t_wall_ns(Pipe_Parameters_t *para)
{
    double wall_speed_mps = 0.0; // 管壁中声速，单位 m/s
    double path_wall_m = 0.0;    // 管壁总传播路径，单位 m
    double t_other_s = 0.0;      // 附加传播时间，单位 s

    if (para == NULL)
    {
        log_e("para pointer don't exist!");
        return 0.0;
    }

    /* 防止除0或非法参数 */
    if (para->cos_value <= 0.0)
    {
        log_e("cos_value is 0!");
        return 0.0;
    }

    if (para->wall_thick <= 0.0)
    {
        log_e("wall_thick is 0!");
        return 0.0;
    }

    /* 根据管材选取一个近似纵波声速
       这里只是工程近似值，后续你可以再标定修正 */
    switch (para->pipe_type)
    {
    case PIPE_PVC:
        wall_speed_mps = 2380.0; // PVC，粗略取值
        break;

    case PIPE_METAL:
        wall_speed_mps = 5900.0; // 钢管，粗略取值
        break;

    case PIPE_ALLOY:
        wall_speed_mps = 4700.0; // 铜/合金，先给个近似值
        break;

    default:
        log_e("pipe_type is wrong!");
        wall_speed_mps = 3000.0; // 默认兜底
        break;
    }

    /* 管壁附加传播路径：
       单侧斜穿长度 = wall_thick / cos
       两侧总长度   = 2 * wall_thick / cos
       wall_thick原单位是 mm，这里转成 m
    */
    path_wall_m = (2.0 * para->wall_thick / para->cos_value) * 1e-3;

    /* 传播时间（秒） */
    t_other_s = path_wall_m / wall_speed_mps;

    /* 转成 ns 返回 */
    return t_other_s * 1e9;
}

/**  根据 dt 计算原始流速 v（m/s）
 * 公式：v = dt * L1 / (cos_sin * (t1 - te) * (t2 - te))
 *
 * - t1_ns / t2_ns / dt_ns : ns
 * - te : us（会转换为 ns）
 * - L1 = pipe_dn : mm
 * - 输出 v : m/s
 */
double vel_calc_from_dt(Pipe_Parameters_t *para,
                        double t1_ns,
                        double t2_ns,
                        double dt_ns)
{
    // ---- 参数准备 ----
    const double cos_sin = para->cos_value * para->sin_value;

    // te：系统固定误差（us -> ns）
    const double te_ns = para->te_ns;
    const double te_wall_ns = calc_t_wall_ns(para); // 管壁传播时间

    // L1：这里直接用 pipe_dn（mm）
    const double L1_mm = para->inner_diameter;

    // ---- 合法性检查 ----
    const double a = t1_ns - te_ns - te_wall_ns;
    const double b = t2_ns - te_ns - te_wall_ns;

    if (cos_sin == 0.0 || a <= 0.0 || b <= 0.0)
    {
        log_e("cos*sin = 0!");
        return 0.0;
    }

    // ---- 计算 ----
    // 单位：(ns * mm) / (ns * ns) = mm/ns
    const double v_mm_per_ns =
        (dt_ns * L1_mm) / (cos_sin * a * b);

    // mm/ns -> m/s
    const double v_mps = v_mm_per_ns * 1e6;

    return v_mps;
}

/**
 * @brief  滑动窗加入新值，并在满足条件时输出 IQR 去异常均值
 * @note
 *  - 窗口长度 FLOW_WINDOW_LEN（30）
 *  - 每 FLOW_WINDOW_STEP（5）次更新输出一次
 *  - 输出时对窗口排序，按箱线图（IQR）剔除离群值，再求平均
 */
bool flow_window_add(Pipe_algo_state_t *state,
                     double *v_raw,
                     double *v_avg)
{
    state->window_buf[state->window_idx] = *v_raw;
    state->window_idx = (uint8_t)((state->window_idx + 1U) % FLOW_WINDOW_LEN);
    if (state->window_idx == 0U)
    {
        state->window_full = true;
    }

    state->step_cnt = (uint8_t)((state->step_cnt + 1U) % FLOW_WINDOW_STEP);
    if (state->step_cnt != 0U)
    {
        return false;
    }
    if (!state->window_full)
    {
        return false;
    }

    double temp[FLOW_WINDOW_LEN];
    memcpy(temp, state->window_buf, sizeof(temp));

    // 简单排序（窗口小，冒泡可接受）
    for (uint8_t i = 0; i < FLOW_WINDOW_LEN - 1U; i++)
    {
        for (uint8_t j = 0; j < FLOW_WINDOW_LEN - 1U - i; j++)
        {
            if (temp[j] > temp[j + 1U])
            {
                double t = temp[j];
                temp[j] = temp[j + 1U];
                temp[j + 1U] = t;
            }
        }
    }

    // Q1/Q3：取第7个和第23个（1-based）
    double q1 = temp[6];
    double q3 = temp[22];
    double iqr = q3 - q1;
    double low = q1 - 1.5 * iqr;
    double high = q3 + 1.5 * iqr;

    double sum = 0.0;
    uint8_t cnt = 0U;
    for (uint8_t i = 0; i < FLOW_WINDOW_LEN; i++)
    {
        if (temp[i] >= low && temp[i] <= high)
        {
            sum += temp[i];
            cnt++;
        }
    }
    if (cnt == 0U)
    {
        return false;
    }

    *v_avg = sum / (double)cnt;
    return true;
}

// 一维卡尔曼滤波（输入 measurement，输出平滑后的估计值）
double run_kalman_filter(kalman_t *k, double measurement)
{
    k->p += k->q;
    k->k = k->p / (k->p + k->r);
    k->x += k->k * (measurement - k->x);
    k->p *= (1.0 - k->k);
    return k->x;
}

// 自动 0漂补偿：仅在 “SQ高 + 输出接近0 + 连续稳定” 时缓慢学习 offset
double flow_drift_comp(Pipe_Parameters_t *para,
                       Pipe_algo_state_t *state,
                       double v,
                       double sq)
{
    // ====== 参数（建议后续调参）======
    const double SQ_LEARN_MIN = para->zero_learn_sq_min;    // SQ阈值：信号质量足够好才学
    const double V_NEAR_ZERO = para->zero_learn_flow_speed; // 近零阈值：认为可能无流（单位同 v）
    const float ALPHA = para->zero_learn_alpha;             // 学习速率：越小越慢越稳
    const float OFFSET_MAX = para->zero_learn_offset_max;   // offset 安全限幅（防学歪）

    // 说明：你现在每 5 组（约40ms）才输出一次 v
    // 1秒大约 25 次输出；如果希望 6 秒后才开始学：STABLE_N ~ 150
    const uint32_t STABLE_N = para->zero_stable_threshold;

    // ====== 判断是否满足“可学习零点”的条件 ======
    if (sq >= SQ_LEARN_MIN && fabs(v) <= V_NEAR_ZERO)
    {
        if (state->zero_stable < 0xFFFF)
            state->zero_stable++;
    }
    else
    {
        state->zero_stable = 0;
    }

    // ====== 连续稳定够久才更新 offset（避免停流瞬态/小流误判）======
    if (state->zero_stable >= STABLE_N)
    {
        // 低通学习：offset 缓慢追踪 v（零流时 v≈offset）
        para->zero_offset_speed = (1.0f - ALPHA) * para->zero_offset_speed + ALPHA * v;

        // 安全限幅
        if (para->zero_offset_speed > OFFSET_MAX)
            para->zero_offset_speed = OFFSET_MAX;
        if (para->zero_offset_speed < -OFFSET_MAX)
            para->zero_offset_speed = -OFFSET_MAX;
    }

    // 输出补偿后的结果
    return v - para->zero_offset_speed;
}

bool zero_learn_manual_start(Pipe_Parameters_t *para,
                             Pipe_algo_state_t *state)
{
    double new_offset = 0.0;

    if ((para == NULL) || (state == NULL))
    {
        return false;
    }

    /* 至少要等窗口有效，避免刚开机就拿不稳定数据做零漂 */
    if (!state->window_full)
    {
        return false;
    }

    /* 手动学习也保留基本保护，避免明显脏数据把 offset 学坏 */
    if (state->last_sq_value < para->zero_learn_sq_min)
    {
        return false;
    }

    if (fabs(state->last_flow_speed_mps) > para->zero_learn_offset_max)
    {
        return false;
    }

    /* 当前输出残差 = 还没补干净的零漂，直接吸收到 offset 里 */
    new_offset = para->zero_offset_speed + state->last_flow_speed_mps;

    if (new_offset > para->zero_learn_offset_max)
    {
        new_offset = para->zero_learn_offset_max;
    }
    if (new_offset < -para->zero_learn_offset_max)
    {
        new_offset = -para->zero_learn_offset_max;
    }

    para->zero_offset_speed = new_offset;
    state->zero_stable = 0U;

    return true;
}

// 最终限幅/死区处理
double flow_limit(Pipe_Parameters_t *para,
                  double v)
{
    if (fabs(v) < para->lower_speed_range)
    {
        g_alarm = ALARM_SPEED_LOWER_LIMIT;
        return 0.0;
    }
    if (fabs(v) > para->upper_speed_range)
    {
        g_alarm = ALARM_SPEED_HIGHER_LIMIT;
        return (v > 0.0) ? para->upper_speed_range : -para->upper_speed_range;
    }
    return v;
}

double pipe_area_m2(const Pipe_Parameters_t *para)
{
    // 如果 pipe_dn 代表内径 mm
    const double D_m = para->inner_diameter * 1e-3;
    const double r = 0.5 * D_m;
    return (double)(M_PI * r * r);
}

void update_flow_outputs(Pipe_Parameters_t *para,
                         Pipe_algo_state_t *state,
                         Pipe_algo_out_data_t *out,
                         double sq,
                         double v_mps)
{
    const double A = pipe_area_m2(para);

    /* 内部基准量：m^3/s */
    const double q_m3ps = v_mps * A;
    const VolumeUnitType total_unit = volume_unit_from_rate_unit(para->rate_unit_type);

    /* 累计流量保持 m^3 */
    state->q_total_m3 += q_m3ps * (double)DT_S;

    out->flow_speed = convert_speed_from_mps(v_mps, para->speed_unit_type);
    out->flow_rate_instant = convert_rate_from_m3ps(q_m3ps, para->rate_unit_type);
    out->flow_rate_total = convert_volume_from_m3(state->q_total_m3, total_unit);
    out->sq_value = sq;
    out->flow_speed_unit = para->speed_unit_type;
    out->flow_rate_unit = para->rate_unit_type;
    out->flow_total_unit = total_unit;
}

void flow_alarm(Pipe_Parameters_t *para,
                double flow_speed_mps)
{
    const double flow_rate_m3ps = flow_speed_mps * pipe_area_m2(para);

    if (flow_rate_m3ps >= para->alarm_upper_rate_range)
    {
        g_alarm = ALARM_RATE_TOO_HIGH;
    }
    else if (flow_rate_m3ps <= para->alarm_lower_rate_range)
    {
        g_alarm = ALARM_RATE_TOO_LOW;
    }
}
