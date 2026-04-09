/*
 * 原始超声数据包解析文件。
 * 本文件负责把采集侧产生的 22 字节原始包拆成 idx 和 48 位回波数据，
 * 再进一步计算算法主链需要的 t1 / t2 / dt。
 */
#include "algorithm_packet.h"
#define LOG_TAG "algo_pkt"
#define LOG_LVL ELOG_LVL_ERROR

#include <math.h>

#include "elog.h"

/* 将 48 位有符号值扩展为 64 位，供后续 double 计算使用。 */
static int64_t sign_extend_48(uint64_t v)
{
    if (v & 0x800000000000ULL)
    {
        return (int64_t)(v | 0xFFFF000000000000ULL);
    }
    return (int64_t)v;
}

/*
 * 拆包函数。
 * 用途：
 * 1. 从原始字节流中取出 idxA / idxB；
 * 2. 解析三个 48 位回波值；
 * 3. 保留序号，供上层检测重复包。
 *
 * 本函数主要被 `task_algorithm()` 调用，是算法处理的第一步。
 */
void rufx_unpack_packet(const rufx_raw_packet_t *raw,
                        rufx_packet_t *out,
                        uint8_t *seq)
{
    const uint8_t *b = raw->bytes;

    out->idx_a = (uint16_t)((uint16_t)b[0] << 8) | (uint16_t)b[1];
    out->idx_b = (uint16_t)((uint16_t)b[2] << 8) | (uint16_t)b[3];

    uint64_t y1 = ((uint64_t)b[4] << 40) | ((uint64_t)b[5] << 32) |
                  ((uint64_t)b[6] << 24) | ((uint64_t)b[7] << 16) |
                  ((uint64_t)b[8] << 8) | (uint64_t)b[9];
    uint64_t y2 = ((uint64_t)b[10] << 40) | ((uint64_t)b[11] << 32) |
                  ((uint64_t)b[12] << 24) | ((uint64_t)b[13] << 16) |
                  ((uint64_t)b[14] << 8) | (uint64_t)b[15];
    uint64_t y3 = ((uint64_t)b[16] << 40) | ((uint64_t)b[17] << 32) |
                  ((uint64_t)b[18] << 24) | ((uint64_t)b[19] << 16) |
                  ((uint64_t)b[20] << 8) | (uint64_t)b[21];

    out->conv_y1 = sign_extend_48(y1);
    out->conv_y2 = sign_extend_48(y2);
    out->conv_y3 = sign_extend_48(y3);

    if (seq != NULL)
    {
        *seq = raw->seq;
    }
}

/*
 * 根据 idx 和抛物线三点插值计算 t1 / t2 / dt。
 * 其中：
 * - t1 / t2 是两个采样点的绝对时间；
 * - dt 是考虑亚采样插值后的飞行时间差。
 *
 * 这一步失败通常意味着回波形状异常或分母接近 0，上层会把这一帧当作无效数据。
 */
bool rufx_calc_t1_t2_dt(const rufx_packet_t *pkt,
                        double *t1,
                        double *t2,
                        double *dt)
{
    const double Ts_ns = 1e9 / 65e6;

    *t1 = (double)pkt->idx_a * Ts_ns;
    *t2 = (double)pkt->idx_b * Ts_ns;

    const double y1 = (double)pkt->conv_y1;
    const double y2 = (double)pkt->conv_y2;
    const double y3 = (double)pkt->conv_y3;

    const double den = (y3 - 2.0 * y2 + y1);
    if (fabs(den) < 1e-6)
    {

        return false;
    }

    const double delta = 0.5 * (y3 - y1) / den;
    if (delta < -1.0 || delta > 1.0)
    {
        return false;
    }

    *dt = (*t2 - *t1) + delta * Ts_ns;
    return true;
}
