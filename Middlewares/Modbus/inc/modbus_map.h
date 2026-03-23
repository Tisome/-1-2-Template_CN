#define MB_COIL_MEASURE_ENABLE          0
#define MB_COIL_CLEAR_TOTALIZER         1
#define MB_COIL_ZERO_LEARN_START        2
#define MB_COIL_SAVE_PARAMETERS         3
#define MB_COIL_LOAD_DEFAULT_PARAMETERS 4
#define MB_COIL_CLEAR_ALARM             5
#define MB_COIL_SOFT_RESET              6

#define HR_INNER_DIAMETER               0
#define HR_WALL_THICK                   1
#define HR_COS_VALUE_H                  2
#define HR_SIN_VALUE_H                  4
#define HR_LOWER_SPEED_RANGE_H          6
#define HR_UPPER_SPEED_RANGE_H          8
#define HR_ALARM_LOWER_RATE_RANGE_H     10
#define HR_ALARM_UPPER_RATE_RANGE_H     12
#define HR_ZERO_OFFSET_SPEED_H          14
#define HR_ZERO_LEARN_FLOW_SPEED_H      16
#define HR_ZERO_LEARN_ALPHA_H           18
#define HR_ZERO_LEARN_OFFSET_MAX_H      20
#define HR_ZERO_LEARN_SQ_MIN_H          22
#define HR_TE_NS_H                      24
#define HR_IS_SAVED                     26
#define HR_OUTPUT_MODE                  27
#define HR_DISPLAY_SENSITIVITY          28
#define HR_ZERO_STABLE_THRESHOLD        29
#define HR_MODBUS_ADDR                  30
#define HR_PIPE_TYPE                    31
#define HR_SPEED_UNIT_TYPE              32
#define HR_RATE_UNIT_TYPE               33
#define HR_LAST_WRITABLE                HR_RATE_UNIT_TYPE

#define IR_FLOW_SPEED_H                 0
#define IR_FLOW_RATE_INSTANT_H          2
#define IR_FLOW_RATE_TOTAL_H            4
#define IR_SQ_VALUE_H                   8

#define IR_ZERO_STABLE                  10
#define IR_SQ_IDX                       11
#define IR_SQ_COUNT                     12
#define IR_SQ_BAD_COUNT                 13
#define IR_WINDOW_IDX                   14
#define IR_STEP_CNT                     15
#define IR_WINDOW_FULL                  16
#define IR_Q_TOTAL_M3_H                 17

#define DI_ALARM_EXIST                  0  // 当前是否有报警
#define DI_ALARM_REPEAT_PACKET          1  // 重包报警
#define DI_ALARM_OUT_OF_TIME            2  // 超时报警
#define DI_ALARM_SPEED_LOWER            3  // 流速低于下限
#define DI_ALARM_SPEED_HIGHER           4  // 流速高于上限
#define DI_ALARM_RATE_LOW               5  // 流量过低
#define DI_ALARM_RATE_HIGH              6  // 流量过高
#define DI_ZERO_STABLE_REACHED          7  // 已满足零漂稳定条件
#define DI_WINDOW_FULL                  8  // 流速窗口已满
#define DI_PARAMETER_SAVED              9  // 参数已保存
#define DI_MEASUREMENT_VALID            10 // 当前测量值有效

// 控制
/*
| Coil地址 | 名称                      | 说明     |
| ------ | ----------------------- | ------ |
| 0      | measure_enable          | 测量使能   |
| 1      | clear_totalizer         | 清零累计流量 |
| 2      | zero_learn_start        | 启动零漂学习 |
| 3      | save_parameters         | 保存参数   |
| 4      | load_default_parameters | 恢复默认参数 |
| 5      | clear_alarm             | 清除报警   |
| 6      | soft_reset              | 软件复位   |
| 7~15   | reserved                | 预留     |
*/

// 状态
/*
| Discrete地址 | 名称                       | 说明        |
| ---------- | ------------------------ | --------- |
| 0          | alarm_exist              | 当前是否有报警   |
| 1          | alarm_repeat_packet      | 重包报警      |
| 2          | alarm_out_of_time        | 超时报警      |
| 3          | alarm_speed_lower_limit  | 流速低于下限    |
| 4          | alarm_speed_higher_limit | 流速高于上限    |
| 5          | alarm_rate_too_low       | 流量过低      |
| 6          | alarm_rate_too_high      | 流量过高      |
| 7          | zero_stable_reached      | 已满足零漂稳定条件 |
| 8          | window_full              | 流速窗口已满    |
| 9          | parameter_saved          | 参数已保存     |
| 10         | measurement_valid        | 当前测量值有效   |
| 11~15      | reserved                 | 预留        |
*/

// 配置参数
/*
| Holding地址 | 名称                     | 来源字段                                  | 类型  | 缩放/说明        |
| --------- | ---------------------- | ------------------------------------- | --- | ------------ |
| 0         | inner_diameter         | `g_parameters.inner_diameter`         | U16 | mm ×100      |
| 1         | wall_thick             | `g_parameters.wall_thick`             | U16 | mm ×100      |
| 2~3       | cos_value              | `g_parameters.cos_value`              | S32 | ×1000000     |
| 4~5       | sin_value              | `g_parameters.sin_value`              | S32 | ×1000000     |
| 6~7       | lower_speed_range      | `g_parameters.lower_speed_range`      | S32 | m/s ×1000    |
| 8~9       | upper_speed_range      | `g_parameters.upper_speed_range`      | S32 | m/s ×1000    |
| 10~11     | alarm_lower_rate_range | `g_parameters.alarm_lower_rate_range` | S32 | 约定基础单位 ×1000 |
| 12~13     | alarm_upper_rate_range | `g_parameters.alarm_upper_rate_range` | S32 | 约定基础单位 ×1000 |
| 14~15     | zero_offset_speed      | `g_parameters.zero_offset_speed`      | S32 | m/s ×1000    |
| 16~17     | zero_learn_flow_speed  | `g_parameters.zero_learn_flow_speed`  | S32 | m/s ×1000    |
| 18~19     | zero_learn_alpha       | `g_parameters.zero_learn_alpha`       | S32 | ×1000000     |
| 20~21     | zero_learn_offset_max  | `g_parameters.zero_learn_offset_max`  | S32 | m/s ×1000    |
| 22~23     | zero_learn_sq_min      | `g_parameters.zero_learn_sq_min`      | S32 | ×1000        |
| 24~25     | te_ns                  | `g_parameters.te_ns`                  | S32 | ns           |
| 26        | is_saved               | `g_parameters.is_saved`               | U16 | 0/1          |
| 27        | output_mode            | `g_parameters.output_mode`            | U16 | 原值           |
| 28        | display_sensitivity    | `g_parameters.display_sensitivity`    | U16 | 原值           |
| 29        | zero_stable_threshold  | `g_parameters.zero_stable_threshold`  | U16 | 原值           |
| 30        | modbus_addr            | `g_parameters.modbus_addr`            | U16 | 原值           |
| 31        | pipe_type              | `g_parameters.pipe_type`              | U16 | 枚举           |
| 32        | speed_unit_type        | `g_parameters.speed_unit_type`        | U16 | 枚举           |
| 33        | rate_unit_type         | `g_parameters.rate_unit_type`         | U16 | 枚举           |
*/

// 实时测量值
/*
实时输出区 g_algo_out

| Input地址 | 名称 | 来源字段 | 类型 | 缩放 / 说明 |
| -- -- -- -| -- -- -- -- -- -- -- -- -| -- -- -- -- -- -- -- -- -- -- -- -- -- -- --| -- -| -- -- -- -- --|
| 0 ~1 | flow_speed | `g_algo_out.flow_speed` | S32 | m / s ×1000 |
| 2 ~3 | flow_rate_instant | `g_algo_out.flow_rate_instant` | S32 | 基础单位 ×1000 |
| 4 ~7 | flow_rate_total | `g_algo_out.flow_rate_total` | S64 | 基础单位 ×1000 |
| 8 ~9 | sq_value | `g_algo_out.sq_value` | S32 | ×1000 |

算法诊断区 g_algo_state

| Input地址 | 名称           | 来源字段                        | 类型  | 说明       |
| ------- | ------------ | --------------------------- | --- | -------- |
| 10      | zero_stable  | `g_algo_state.zero_stable`  | U16 | 原值       |
| 11      | sq_idx       | `g_algo_state.sq_idx`       | U16 | 原值       |
| 12      | sq_count     | `g_algo_state.sq_count`     | U16 | 原值       |
| 13      | sq_bad_count | `g_algo_state.sq_bad_count` | U16 | 原值       |
| 14      | window_idx   | `g_algo_state.window_idx`   | U16 | 原值       |
| 15      | step_cnt     | `g_algo_state.step_cnt`     | U16 | 原值       |
| 16      | window_full  | `g_algo_state.window_full`  | U16 | 0/1      |
| 17~20   | q_total_m3   | `g_algo_state.q_total_m3`   | S64 | m³ ×1000 |

*/
