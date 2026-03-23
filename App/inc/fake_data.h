#ifndef __FAKE_DATA_H__
#define __FAKE_DATA_H__

#include "data.h"

typedef struct
{
    float lower;    /* 模式为 SPEED 时单位 m/s；模式为 FLOW 时单位 L/min */
    float upper;    /* 同上 */
    float period_s; /* 周期，秒 */
} fake_data_cfg_t;

void fake_data_set_cfg(float lower, float upper, float period_s);
void fake_data_get_cfg(fake_data_cfg_t *cfg);

float fake_data_get_target_speed_mps(float t_s, const Pipe_Parameters_t *para);
void fake_data_make_packet(rufx_raw_packet_t *raw, float t_s, const Pipe_Parameters_t *para);

#endif
