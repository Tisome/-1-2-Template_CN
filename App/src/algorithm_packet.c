#include "algorithm_packet.h"
#include <math.h>

#include "elog.h"
#define LOG_TAG "algo_packet"
#define LOG_LVL ELOG_LVL_VERBOSE

static int64_t sign_extend_48(uint64_t v)
{
    if (v & 0x800000000000ULL)
    {
        return (int64_t)(v | 0xFFFF000000000000ULL);
    }
    return (int64_t)v;
}

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

    *seq = raw->seq;
}

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