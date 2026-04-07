/*
 * 假数据底层构造文件。
 * 与 `task_fake_data.c` 不同，本文件不直接跑任务，而是提供：
 * 1. 假数据配置管理
 * 2. 目标波形生成
 * 3. 原始数据包打包
 *
 * 它相当于“测试数据源的算法层”，上层任务只负责定时调用。
 */
#include "fake_data.h"
#include "algorithm_flow.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FAKE_IDX_A 3000U

static fake_data_cfg_t g_fake_data_cfg =
    {
#if FAKE_DATA_MODE == FAKE_DATA_MODE_SPEED
        .lower = 1.0f, /* m/s */
        .upper = 4.0f, /* m/s */
        .period_s = 10.0f
#elif FAKE_DATA_MODE == FAKE_DATA_MODE_FLOW
        .lower = 10.0f, /* L/min */
        .upper = 40.0f, /* L/min */
        .period_s = 10.0f
#endif
};

static volatile uint8_t g_fake_data_cfg_refresh_pending = 0U;

/* ========================= 基础工具函数 ========================= */

/* 生成一个随时间变化的正弦目标值，用于模拟周期性流速/流量变化。 */
static float fake_sine_wave(float t_s, const fake_data_cfg_t *cfg)
{
    float mid = 0.5f * (cfg->lower + cfg->upper);
    float amp = 0.5f * (cfg->upper - cfg->lower);
    float period = (cfg->period_s > 0.001f) ? cfg->period_s : 0.001f;
    float w = 2.0f * (float)M_PI / period;

    return mid + amp * sinf(w * t_s);
}

#if FAKE_DATA_MODE == FAKE_DATA_MODE_FLOW
/* 根据内径计算横截面积，仅在按流量模式生成假数据时使用。 */
static double pipe_area_m2(const Pipe_Parameters_t *para)
{
    if (para == NULL || para->inner_diameter <= 0.0)
    {
        return 0.0;
    }

    double D_m = para->inner_diameter * 1e-3; /* mm -> m */
    double r_m = 0.5 * D_m;
    return M_PI * r_m * r_m;
}

/* L/min -> m^3/s */
/* 将 L/min 转换为 m^3/s。 */
static double lpm_to_m3ps(double q_lpm)
{
    return q_lpm * 1e-3 / 60.0;
}

#endif

/* 将 64 位整数截取为 48 位，准备写入模拟原始包。 */
static uint64_t pack_s48(int64_t v)
{
    return (uint64_t)v & 0xFFFFFFFFFFFFULL;
}

// big endian
/* 以大端格式写入一个 48 位有符号值。 */
static void put_be48(uint8_t *p, int64_t s48)
{
    uint64_t u = pack_s48(s48);

    p[0] = (uint8_t)((u >> 40) & 0xFF);
    p[1] = (uint8_t)((u >> 32) & 0xFF);
    p[2] = (uint8_t)((u >> 24) & 0xFF);
    p[3] = (uint8_t)((u >> 16) & 0xFF);
    p[4] = (uint8_t)((u >> 8) & 0xFF);
    p[5] = (uint8_t)(u & 0xFF);
}

/* 让抛物线插值精确回到 delta */
static void make_parabola_3pts(double delta, int64_t *y1, int64_t *y2, int64_t *y3)
{
    const double A = 1e9;
    const double K = 1e8;

    double r0 = A - K * (0.0 - delta) * (0.0 - delta);
    double rp1 = A - K * (1.0 - delta) * (1.0 - delta);   /* y1 = R(+1) */
    double rm1 = A - K * (-1.0 - delta) * (-1.0 - delta); /* y3 = R(-1) */

    *y1 = (int64_t)llround(rp1);
    *y2 = (int64_t)llround(r0);
    *y3 = (int64_t)llround(rm1);
}

/* ========================= 对外接口 ========================= */

/* 设置假数据波形范围和周期。 */
void fake_data_set_cfg(float lower, float upper, float period_s)
{
    g_fake_data_cfg.lower = lower;
    g_fake_data_cfg.upper = upper;
    g_fake_data_cfg.period_s = period_s;
}

/* 读取当前假数据配置。 */
void fake_data_get_cfg(fake_data_cfg_t *cfg)
{
    if (cfg == NULL)
    {
        return;
    }

    *cfg = g_fake_data_cfg;
}

/* 由外部模块请求刷新假数据配置，通常在参数变化后调用。 */
void fake_data_request_cfg_refresh(void)
{
    g_fake_data_cfg_refresh_pending = 1U;
}

/* 取走一次“需要刷新配置”的标志，供任务侧决定是否重新生成范围。 */
bool fake_data_consume_cfg_refresh_request(void)
{
    if (g_fake_data_cfg_refresh_pending == 0U)
    {
        return false;
    }

    g_fake_data_cfg_refresh_pending = 0U;
    return true;
}

/* 根据当前时间点计算目标流速，统一输出为 m/s。 */
float fake_data_get_target_speed_mps(float t_s, const Pipe_Parameters_t *para)
{
    float signal = fake_sine_wave(t_s, &g_fake_data_cfg);

#if FAKE_DATA_MODE == FAKE_DATA_MODE_SPEED
    /* 直接就是 m/s */
    return signal;

#elif FAKE_DATA_MODE == FAKE_DATA_MODE_FLOW
    /* signal 单位 L/min，先转 m^3/s，再除面积得到 m/s */
    double q_m3ps = lpm_to_m3ps((double)signal);
    double area_m2 = pipe_area_m2(para);

    if (area_m2 <= 0.0)
    {
        return 0.0f;
    }

    return (float)(q_m3ps / area_m2);
#else
    return 0.0f;
#endif
}

/*
 * 生成一帧模拟原始包。
 * 本函数会先根据当前目标流速反推 dt，再把 idx、三点回波数据封装成与真实采集一致的原始格式，
 * 因此上层算法无需区分“是真实数据还是假数据”。
 */
void fake_data_make_packet(rufx_raw_packet_t *raw,
                           float t_s,
                           const Pipe_Parameters_t *para)
{
    if (raw == NULL || para == NULL)
    {
        return;
    }

    memset(raw, 0, sizeof(*raw));

    const double Ts_ns = 1e9 / 65e6;
    const uint16_t idx_a = FAKE_IDX_A;

    /* 当前目标流速 */
    double v_mps = (double)fake_data_get_target_speed_mps(t_s, para);

    /* ---- 参数 ---- */

    double t1_ns = (double)idx_a * Ts_ns;

    double te_ns = para->te_ns;

    /* 新增：管壁传播时间 */
    double t_wall_ns = calc_t_wall_ns(para);

    double cos_sin = para->cos_value * para->sin_value;

    double L1_mm = para->inner_diameter;

    /* ---- 与算法完全一致的模型 ---- */

    double a = t1_ns - te_ns - t_wall_ns;

    if (a < 1000.0)
    {
        a = 1000.0;
    }

    /* 近似 b ≈ a */
    double b = a;

    if (cos_sin == 0.0 || L1_mm <= 0.0)
    {
        return;
    }

    /* v(m/s) -> mm/ns */
    double v_mm_per_ns = v_mps * 1e-6;

    /* 反推 dt */
    double dt_ns =
        v_mm_per_ns * (cos_sin * a * b) / L1_mm;

    /* ---- dt -> sample ---- */

    double d = dt_ns / Ts_ns;

    int di = (int)floor(d + 0.5);

    double delta = d - (double)di;

    // 偏移限制
    if (delta < -0.95)
        delta = -0.95;
    if (delta > 0.95)
        delta = 0.95;

    uint16_t idx_b = (uint16_t)(idx_a + di);

    /* ---- 构造抛物线三点 ---- */

    int64_t y1, y2, y3;

    make_parabola_3pts(delta, &y1, &y2, &y3);

    /* ---- 打包 raw packet ---- */

    raw->bytes[0] = (uint8_t)(idx_a >> 8);
    raw->bytes[1] = (uint8_t)(idx_a);

    raw->bytes[2] = (uint8_t)(idx_b >> 8);
    raw->bytes[3] = (uint8_t)(idx_b);

    put_be48(&raw->bytes[4], y1);
    put_be48(&raw->bytes[10], y2);
    put_be48(&raw->bytes[16], y3);
}
