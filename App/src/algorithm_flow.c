#include "algorithm_flow.h"
#define LOG_TAG "algo_flow"
#define LOG_LVL ELOG_LVL_ERROR

#include <math.h>
#include <string.h>

#include "elog.h"

// 鏇存柊 3 绉掔獥鍙ｅ潖鏁版嵁缁熻锛堢幆褰㈢紦鍐诧級
/* 更新 SQ 坏点统计窗口，窗口长度由 `SQ_WINDOW_GROUPS` 决定。 */
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

    // 绐楀彛宸叉弧锛氳鐩栨渶鏃ф暟鎹?
    s->sq_bad_count -= s->bad_flags[s->sq_idx];
    s->bad_flags[s->sq_idx] = is_bad ? 1U : 0U;
    s->sq_bad_count += is_bad ? 1U : 0U;
    s->sq_idx = (uint16_t)((s->sq_idx + 1U) % SQ_WINDOW_GROUPS);
}

// 鑾峰彇 SQ锛堝ソ鏁版嵁鐧惧垎姣旓級
/* 计算当前 SQ 百分比，本质上是“好数据比例”。 */
double sq_get_percent(const Pipe_algo_state_t *s)
{
    uint16_t denom = (s->sq_count == 0U) ? 1U : s->sq_count;
    double bad = s->sq_bad_count;
    double total = (double)denom;
    return 100.0 * (1.0 - (bad / total));
}

/* 估算声波穿过管壁带来的附加传播时间，返回单位为 ns。 */
double calc_t_wall_ns(const Pipe_Parameters_t *para)
{
    double wall_speed_mps = 0.0; // 绠″涓０閫燂紝鍗曚綅 m/s
    double path_wall_m = 0.0;    // 绠″鎬讳紶鎾矾寰勶紝鍗曚綅 m
    double t_other_s = 0.0;      // 闄勫姞浼犳挱鏃堕棿锛屽崟浣?s

    if (para == NULL)
    {
        log_e("para pointer don't exist!");
        return 0.0;
    }

    /* 闃叉闄?鎴栭潪娉曞弬鏁?*/
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

    /* 鏍规嵁绠℃潗閫夊彇涓€涓繎浼肩旱娉㈠０閫?
       杩欓噷鍙槸宸ョ▼杩戜技鍊硷紝鍚庣画浣犲彲浠ュ啀鏍囧畾淇 */
    switch (para->pipe_type)
    {
    case PIPE_PVC:
        wall_speed_mps = 2380.0; // PVC锛岀矖鐣ュ彇鍊?
        break;

    case PIPE_METAL:
        wall_speed_mps = 5900.0; // 閽㈢锛岀矖鐣ュ彇鍊?
        break;

    case PIPE_ALLOY:
        wall_speed_mps = 4700.0; // 閾?鍚堥噾锛屽厛缁欎釜杩戜技鍊?
        break;

    default:
        log_e("pipe_type is wrong!");
        wall_speed_mps = 3000.0; // 榛樿鍏滃簳
        break;
    }

    /* 绠″闄勫姞浼犳挱璺緞锛?
       鍗曚晶鏂滅┛闀垮害 = wall_thick / cos
       涓や晶鎬婚暱搴?  = 2 * wall_thick / cos
       wall_thick鍘熷崟浣嶆槸 mm锛岃繖閲岃浆鎴?m
    */
    path_wall_m = (2.0 * para->wall_thick / para->cos_value) * 1e-3;

    /* 浼犳挱鏃堕棿锛堢锛?*/
    t_other_s = path_wall_m / wall_speed_mps;

    /* 杞垚 ns 杩斿洖 */
    return t_other_s * 1e9;
}

/**  鏍规嵁 dt 璁＄畻鍘熷娴侀€?v锛坢/s锛?
 * 鍏紡锛歷 = dt * L1 / (cos_sin * (t1 - te) * (t2 - te))
 *
 * - t1_ns / t2_ns / dt_ns : ns
 * - te : us锛堜細杞崲涓?ns锛?
 * - L1 = pipe_dn : mm
 * - 杈撳嚭 v : m/s
 */
/*
 * 根据时延差计算原始流速，输出单位为 m/s。
 * 主要被 `algorithm_process_group()` 调用，是算法链中最核心的物理换算步骤之一。
 */
double vel_calc_from_dt(const Pipe_Parameters_t *para,
                        double t1_ns,
                        double t2_ns,
                        double dt_ns)
{
    if (para == NULL)
    {
        log_e("para pointer don't exist!");
        return 0.0;
    }
    // ---- 鍙傛暟鍑嗗 ----
    const double cos_sin = para->cos_value * para->sin_value;

    // te锛氱郴缁熷浐瀹氳宸紙us -> ns锛?
    const double te_ns = para->te_ns;
    const double te_wall_ns = calc_t_wall_ns(para); // 绠″浼犳挱鏃堕棿

    // L1锛氳繖閲岀洿鎺ョ敤 pipe_dn锛坢m锛?
    const double L1_mm = para->inner_diameter;

    // ---- 鍚堟硶鎬ф鏌?----
    const double a = t1_ns - te_ns - te_wall_ns;
    const double b = t2_ns - te_ns - te_wall_ns;

    if (cos_sin == 0.0 || a <= 0.0 || b <= 0.0)
    {
        log_e("cos*sin = 0!");
        return 0.0;
    }

    // ---- 璁＄畻 ----
    // 鍗曚綅锛?ns * mm) / (ns * ns) = mm/ns
    const double v_mm_per_ns =
        (dt_ns * L1_mm) / (cos_sin * a * b);

    // mm/ns -> m/s
    const double v_mps = v_mm_per_ns * 1e6;

    return v_mps;
}

/**
 * @brief  婊戝姩绐楀姞鍏ユ柊鍊硷紝骞跺湪婊¤冻鏉′欢鏃惰緭鍑?IQR 鍘诲紓甯稿潎鍊?
 * @note
 *  - 绐楀彛闀垮害 FLOW_WINDOW_LEN锛?0锛?
 *  - 姣?FLOW_WINDOW_STEP锛?锛夋鏇存柊杈撳嚭涓€娆?
 *  - 杈撳嚭鏃跺绐楀彛鎺掑簭锛屾寜绠辩嚎鍥撅紙IQR锛夊墧闄ょ缇ゅ€硷紝鍐嶆眰骞冲潎
 */
/*
 * 滑动窗口平均与去异常函数。
 * 它并不是每次调用都输出结果，而是先把新值写入窗口：
 * 1. 只有窗口填满后才允许输出；
 * 2. 只有累计到 `FLOW_WINDOW_STEP` 次更新后才做一次统计；
 * 3. 统计时先排序，再用 IQR 方法剔除离群值，最后求平均。
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

    // 绠€鍗曟帓搴忥紙绐楀彛灏忥紝鍐掓场鍙帴鍙楋級
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

    // Q1/Q3锛氬彇绗?涓拰绗?3涓紙1-based锛?
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

// 涓€缁村崱灏旀浖婊ゆ尝锛堣緭鍏?measurement锛岃緭鍑哄钩婊戝悗鐨勪及璁″€硷級
/* 一维卡尔曼滤波，用于进一步平滑流速结果。 */
double run_kalman_filter(kalman_t *k, double measurement)
{
    k->p += k->q;
    k->k = k->p / (k->p + k->r);
    k->x += k->k * (measurement - k->x);
    k->p *= (1.0 - k->k);
    return k->x;
}

// 鑷姩 0婕傝ˉ鍋匡細浠呭湪 鈥淪Q楂?+ 杈撳嚭鎺ヨ繎0 + 杩炵画绋冲畾鈥?鏃剁紦鎱㈠涔?offset
/*
 * 自动零漂补偿。
 * 只有在“SQ 足够高、流速接近零、并且连续稳定足够久”时才缓慢更新零点偏移，
 * 这样可以减小长期零漂，又避免在真正有流量时把偏移学坏。
 */
double flow_drift_comp(Pipe_Parameters_t *para,
                       Pipe_algo_state_t *state,
                       double v,
                       double sq)
{
    // ====== 鍙傛暟锛堝缓璁悗缁皟鍙傦級======
    const double SQ_LEARN_MIN = para->zero_learn_sq_min;    // SQ闃堝€硷細淇″彿璐ㄩ噺瓒冲濂芥墠瀛?
    const double V_NEAR_ZERO = para->zero_learn_flow_speed; // 杩戦浂闃堝€硷細璁や负鍙兘鏃犳祦锛堝崟浣嶅悓 v锛?
    const float ALPHA = para->zero_learn_alpha;             // 瀛︿範閫熺巼锛氳秺灏忚秺鎱㈣秺绋?
    const float OFFSET_MAX = para->zero_learn_offset_max;   // offset 瀹夊叏闄愬箙锛堥槻瀛︽锛?

    // 璇存槑锛氫綘鐜板湪姣?5 缁勶紙绾?0ms锛夋墠杈撳嚭涓€娆?v
    // 1绉掑ぇ绾?25 娆¤緭鍑猴紱濡傛灉甯屾湜 6 绉掑悗鎵嶅紑濮嬪锛歋TABLE_N ~ 150
    const uint32_t STABLE_N = para->zero_stable_threshold;

    // ====== 鍒ゆ柇鏄惁婊¤冻鈥滃彲瀛︿範闆剁偣鈥濈殑鏉′欢 ======
    if (sq >= SQ_LEARN_MIN && fabs(v) <= V_NEAR_ZERO)
    {
        if (state->zero_stable < 0xFFFF)
            state->zero_stable++;
    }
    else
    {
        state->zero_stable = 0;
    }

    // ====== 杩炵画绋冲畾澶熶箙鎵嶆洿鏂?offset锛堥伩鍏嶅仠娴佺灛鎬?灏忔祦璇垽锛?=====
    if (state->zero_stable >= STABLE_N)
    {
        // 浣庨€氬涔狅細offset 缂撴參杩借釜 v锛堥浂娴佹椂 v鈮坥ffset锛?
        para->zero_offset_speed = (1.0f - ALPHA) * para->zero_offset_speed + ALPHA * v;

        // 瀹夊叏闄愬箙
        if (para->zero_offset_speed > OFFSET_MAX)
            para->zero_offset_speed = OFFSET_MAX;
        if (para->zero_offset_speed < -OFFSET_MAX)
            para->zero_offset_speed = -OFFSET_MAX;
    }

    // 杈撳嚭琛ュ伩鍚庣殑缁撴灉
    return v - para->zero_offset_speed;
}

/* 手动零漂学习，通常由 GUI 或 Modbus 命令触发。 */
bool zero_learn_manual_start(Pipe_Parameters_t *para,
                             Pipe_algo_state_t *state)
{
    double new_offset = 0.0;

    if ((para == NULL) || (state == NULL))
    {
        return false;
    }

    /* 鑷冲皯瑕佺瓑绐楀彛鏈夋晥锛岄伩鍏嶅垰寮€鏈哄氨鎷夸笉绋冲畾鏁版嵁鍋氶浂婕?*/
    if (!state->window_full)
    {
        return false;
    }

    /* 鎵嬪姩瀛︿範涔熶繚鐣欏熀鏈繚鎶わ紝閬垮厤鏄庢樉鑴忔暟鎹妸 offset 瀛﹀潖 */
    if (state->last_sq_value < para->zero_learn_sq_min)
    {
        return false;
    }

    if (fabs(state->last_flow_speed_mps) > para->zero_learn_offset_max)
    {
        return false;
    }

    /* 褰撳墠杈撳嚭娈嬪樊 = 杩樻病琛ュ共鍑€鐨勯浂婕傦紝鐩存帴鍚告敹鍒?offset 閲?*/
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

// 鏈€缁堥檺骞?姝诲尯澶勭悊
/* 对最终流速做死区和上限处理，同时更新相应流速报警。 */
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

/* 由管道内径计算横截面积，供流速与体积流量互相换算。 */
double pipe_area_m2(const Pipe_Parameters_t *para)
{
    // 濡傛灉 pipe_dn 浠ｈ〃鍐呭緞 mm
    const double D_m = para->inner_diameter * 1e-3;
    const double r = 0.5 * D_m;
    return (double)(M_PI * r * r);
}

/*
 * 根据最终流速更新输出结构。
 * 这里会同时更新瞬时流量、累计流量、SQ 和对外显示单位。
 */
void update_flow_outputs(Pipe_Parameters_t *para,
                         Pipe_algo_state_t *state,
                         Pipe_algo_out_data_t *out,
                         double sq,
                         double v_mps)
{
    const double A = pipe_area_m2(para);

    /* 鍐呴儴鍩哄噯閲忥細m^3/s */
    const double q_m3ps = v_mps * A;
    const VolumeUnitType total_unit = volume_unit_from_rate_unit(para->rate_unit_type);

    /* 绱娴侀噺淇濇寔 m^3 */
    state->q_total_m3 += q_m3ps * (double)DT_S;

    out->flow_speed = convert_speed_from_mps(v_mps, para->speed_unit_type);
    out->flow_rate_instant = convert_rate_from_m3ps(q_m3ps, para->rate_unit_type);
    out->flow_rate_total = convert_volume_from_m3(state->q_total_m3, total_unit);
    out->sq_value = sq;
    out->flow_speed_unit = para->speed_unit_type;
    out->flow_rate_unit = para->rate_unit_type;
    out->flow_total_unit = total_unit;
}

/* 按当前配置的流量上下限判断流量报警。 */
void flow_alarm(Pipe_Parameters_t *para,
                double flow_speed_mps)
{
    double flow_rate_m3ps = 0.0;
    double flow_rate_display = 0.0;

    if (para == NULL)
    {
        return;
    }

    flow_rate_m3ps = flow_speed_mps * pipe_area_m2(para);
    flow_rate_display = fabs(convert_rate_from_m3ps(flow_rate_m3ps, para->rate_unit_type));

    if (flow_rate_display >= para->alarm_upper_rate_range)
    {
        g_alarm = ALARM_RATE_TOO_HIGH;
    }
    else if (flow_rate_display <= para->alarm_lower_rate_range)
    {
        g_alarm = ALARM_RATE_TOO_LOW;
    }
}


/*
 * 流量算法核心函数文件。
 * 本文件集中放置算法处理中最关键的数学与状态更新逻辑，包括：
 * 1. SQ 统计窗口
 * 2. 壁传播时间与流速换算
 * 3. 滑动窗口去异常平均
 * 4. 卡尔曼滤波
 * 5. 自动/手动零漂学习
 * 6. 流速限幅、流量输出与报警
 */
