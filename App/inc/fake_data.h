#ifndef __FAKE_DATA_H__
#define __FAKE_DATA_H__

#include "data.h"

typedef struct
{
    float lower;               /* SPEED 模式下为 m/s；FLOW 模式下为 `flow_unit_type` 对应流量单位 */
    float upper;               /* 同上 */
    float period_s;            /* 周期，秒 */
    RateUnitType flow_unit_type; /* FLOW 模式下配置量纲；SPEED 模式下仅用于日志说明 */
} fake_data_cfg_t;

void fake_data_set_cfg(float lower, float upper, float period_s, RateUnitType flow_unit_type);
void fake_data_get_cfg(fake_data_cfg_t *cfg);
void fake_data_request_cfg_refresh(void);
bool fake_data_consume_cfg_refresh_request(void);

float fake_data_get_target_speed_mps(float t_s, const Pipe_Parameters_t *para);
void fake_data_make_packet(rufx_raw_packet_t *raw, float t_s, const Pipe_Parameters_t *para);

#endif
