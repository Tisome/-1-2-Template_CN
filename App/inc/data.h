#ifndef DATA_H
#define DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_config.h"
#include "at24cxx_handler.h"

#include <stdbool.h>
#include <stdint.h>

/* ========================= 数学常量 ========================= */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ========================= 采样与时间配置 ========================= */

/* 一组数据周期：8ms */
#define GROUP_PERIOD_MS 8U

/* 每秒组数：1000 / 8 = 125 */
#define GROUPS_PER_SEC (1000U / GROUP_PERIOD_MS)

/* SQ 滑动窗口长度：3 秒 */
#define SQ_WINDOW_GROUPS (3U * GROUPS_PER_SEC)

/* 当前输出周期：每 5 组输出一次，即约 40ms */
#define DT_S (0.04)

/* ========================= 阈值配置 ========================= */
/* 注意：t1 / t2 / dt 单位均为 ns */

#define T1_T2_LIMIT_NS  150000.0 /* 150us */
#define DT_UP_LIMIT_NS  1500.0
#define DT_LOW_LIMIT_NS (-25.0)

/* ========================= 滤波配置 ========================= */

#define FLOW_WINDOW_LEN  30U
#define FLOW_WINDOW_STEP 5U

/* ========================= 数据包配置 ========================= */

#define RUF_X_PACKET_FILTER_IDX_SIZE_BYTES 2U
#define RUF_X_PACKET_FILTER_Y_SIZE_BYTES   6U

/* idxA(2) + idxB(2) + y1(6) + y2(6) + y3(6) = 22 bytes */
#define RUF_X_PACKET_SIZE_BYTES \
    (RUF_X_PACKET_FILTER_IDX_SIZE_BYTES * 2U + RUF_X_PACKET_FILTER_Y_SIZE_BYTES * 3U)

/* ========================= 卡尔曼滤波结构 ========================= */

typedef struct
{
    double x; /* 状态估计 */
    double p; /* 估计协方差 */
    double q; /* 过程噪声 */
    double r; /* 测量噪声 */
    double k; /* 卡尔曼增益 */
} kalman_t;

/* ========================= 基础枚举类型 ========================= */

typedef enum {
    PIPE_PVC   = 0, /* PVC 管道 */
    PIPE_METAL = 1, /* 钢管 */
    PIPE_ALLOY = 2  /* 铜管 / 合金管 */
} PipeType;

typedef enum {
    SPEED_UNIT_M_P_S = 0,
    SPEED_UNIT_CM_P_S,
    SPEED_UNIT_MM_P_S
} SpeedUnitType;

typedef enum {
    RATE_UNIT_M3_P_H = 0,
    RATE_UNIT_M3_P_MIN,
    RATE_UNIT_M3_P_S,
    RATE_UNIT_L_P_H,
    RATE_UNIT_L_P_MIN,
    RATE_UNIT_L_P_S
} RateUnitType;

typedef enum {
    VOLUME_UNIT_M3 = 0,
    VOLUME_UNIT_L
} VolumeUnitType;

typedef enum {
    ALARM_OK = 0,
    ALARM_REPEAT_PACKET,
    ALARM_OUT_OF_TIME,
    ALARM_SPEED_LOWER_LIMIT,
    ALARM_SPEED_HIGHER_LIMIT,
    ALARM_RATE_TOO_LOW,
    ALARM_RATE_TOO_HIGH
} ALARM_TYPE;

/* ========================= 参数结构体 ========================= */

typedef struct
{
    /* -------- 几何 / 物理参数（double）-------- */
    double inner_diameter; /* 内径，mm */
    double wall_thick;     /* 管壁厚度，mm */
    double cos_value;      /* 声束角余弦 */
    double sin_value;      /* 声束角正弦 */

    /* -------- 流速限制参数 -------- */
    double lower_speed_range; /* 流速下限，小于此值按 0 处理 */
    double upper_speed_range; /* 流速上限，大于此值报警/限幅 */

    /* -------- 流量报警参数 -------- */
    double alarm_lower_rate_range; /* 流量报警下限 */
    double alarm_upper_rate_range; /* 流量报警上限 */

    /* -------- 零漂学习参数 -------- */
    double zero_offset_speed;     /* 当前零漂补偿量 */
    double zero_learn_flow_speed; /* 零漂学习流速阈值 */
    double zero_learn_alpha;      /* 零漂学习速率 */
    double zero_learn_offset_max; /* 零漂补偿最大限幅 */
    double zero_learn_sq_min;     /* 零漂学习最小 SQ 阈值 */

    /* -------- 系统时延参数 -------- */
    double te_ns; /* 系统固定误差，ns */

    /* -------- 状态 / 配置参数（uint32_t）-------- */
    uint32_t is_saved;              /* 参数是否已保存 */
    uint32_t output_mode;           /* 输出模式 */
    uint32_t display_sensitivity;   /* 显示/上传刷新频率 */
    uint32_t zero_stable_threshold; /* 零漂学习所需连续稳定次数 */

#if USE_MODBUS
    /* -------- Modbus 参数 -------- */
    uint8_t modbus_addr;
#endif

    /* -------- 枚举配置 -------- */
    PipeType pipe_type;            /* 管道类型 */
    SpeedUnitType speed_unit_type; /* 流速单位 */
    RateUnitType rate_unit_type;   /* 流量单位 */

} Pipe_Parameters_t;

/* ========================= 算法运行状态结构体 ========================= */

typedef struct
{
    uint16_t zero_stable; /* 满足零漂学习条件的连续次数 */

    /* SQ 统计窗口 */
    uint8_t bad_flags[SQ_WINDOW_GROUPS]; /* 环形缓冲：1=坏数据，0=好数据 */
    uint16_t sq_idx;                     /* 当前写入位置 */
    uint16_t sq_count;                   /* 当前累计样本数 */
    uint16_t sq_bad_count;               /* 当前坏数据数 */

    /* 流速滑动窗口 */
    double window_buf[FLOW_WINDOW_LEN]; /* 流速窗口数据 */
    uint8_t window_idx;                 /* 当前写入位置 */
    uint8_t step_cnt;                   /* 步进计数 */
    bool window_full;                   /* 窗口是否填满 */

    /* 累计流量 */
    double q_total_m3; /* 累计流量，m^3 */

    /* 最近一次有效输出，内部统一标准单位 */
    double last_flow_speed_mps; /* 最近一次输出流速，m/s */
    double last_sq_value;       /* 最近一次 SQ，% */
} Pipe_algo_state_t;

/* ========================= 算法输出结构体 ========================= */

typedef struct
{
    double flow_speed;        /* 流速 */
    double flow_rate_instant; /* 瞬时流量 */
    double flow_rate_total;   /* 累计流量 */
    double sq_value;          /* SQ 值 */
    SpeedUnitType flow_speed_unit;  /* 流速显示单位 */
    RateUnitType flow_rate_unit;    /* 瞬时流量显示单位 */
    VolumeUnitType flow_total_unit; /* 累计流量显示单位 */
} Pipe_algo_out_data_t;

/* ========================= 数据包结构体 ========================= */

/* 解析后的数据包 */
typedef struct
{
    uint16_t idx_a;
    uint16_t idx_b;
    int64_t conv_y1;
    int64_t conv_y2;
    int64_t conv_y3;
} rufx_packet_t;

/* 原始数据包 */
typedef struct
{
    uint8_t bytes[RUF_X_PACKET_SIZE_BYTES];
    uint8_t seq;
} rufx_raw_packet_t;

/* ========================= 全局变量声明 ========================= */

extern kalman_t kf;

extern Pipe_Parameters_t g_parameters;
extern Pipe_algo_state_t g_algo_state;
extern Pipe_algo_out_data_t g_algo_out;

extern ALARM_TYPE g_alarm;

/* ========================= 接口声明 ========================= */

void parameter_init(void);
void parameter_reset_to_default(void);

double convert_speed_from_mps(double speed_mps, SpeedUnitType unit);
double convert_rate_from_m3ps(double rate_m3ps, RateUnitType unit);
double convert_volume_from_m3(double volume_m3, VolumeUnitType unit);
VolumeUnitType volume_unit_from_rate_unit(RateUnitType rate_unit);

const char *speed_unit_to_str(SpeedUnitType unit);
const char *rate_unit_to_str(RateUnitType unit);
const char *volume_unit_to_str(VolumeUnitType unit);

e2prom_status_t SaveParameters(Pipe_Parameters_t *para);

e2prom_status_t LoadParameters(Pipe_Parameters_t *para);

#ifdef __cplusplus
}
#endif

#endif /* DATA_H */
