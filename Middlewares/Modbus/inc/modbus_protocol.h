#ifndef __MODBUS_PROTOCOL_H
#define __MODBUS_PROTOCOL_H

#include "data.h"

extern Pipe_Parameters_t g_parameters;

#if USE_MODBUS
#define MODBUS_SLAVE_ADDR (g_parameters.modbus_addr)
#else
#define MODBUS_SLAVE_ADDR 0x01
#endif

// Coils（线圈 控制）           → bit   → 可读可写
// Discrete Inputs（离散输入 状态） → bit   → 只读
// Holding Registers（保持寄存器） → 16bit → 可读可写
// Input Registers（输入寄存器）   → 16bit → 只读

// Modbus 功能码

// | addr | 01 | start(2B) | quantity(2B) | CRC | -> 请求
// | addr | 01 | byte_count | data(NB) | CRC | -> 响应
#define MODBUS_FUNC_READ_COILS 0x01 // 读线圈

// | addr | 02 | start(2B) | quantity(2B) | CRC | -> 请求
// | addr | 02 | byte_count | data(NB) | CRC | -> 响应
#define MODBUS_FUNC_READ_DISCRETE_INPUTS 0x02 // 读离散输入

// | addr | 03 | start(2B) | quantity(2B) | CRC | -> 请求
// | addr | 03 | byte_count | data(2N) | CRC | -> 响应
#define MODBUS_FUNC_READ_HOLDING_REGISTERS 0x03 // 读保持寄存器

// | addr | 04 | start(2B) | quantity(2B) | CRC | -> 请求
// | addr | 04 | byte_count | data(2N) | CRC | -> 响应
#define MODBUS_FUNC_READ_INPUT_REGISTERS 0x04 // 读输入寄存器

// | addr | 05 | addr(2B) | value(2B) | CRC | -> 请求
// | addr | 05 | addr(2B) | value(2B) | CRC | -> 响应（回显）
#define MODBUS_FUNC_WRITE_SINGLE_COIL 0x05 // 写单个线圈

// | addr | 06 | addr(2B) | value(2B) | CRC | -> 请求
// | addr | 06 | addr(2B) | value(2B) | CRC | -> 响应（回显）
#define MODBUS_FUNC_WRITE_SINGLE_REGISTER 0x06 // 写单个寄存器

// | addr | 10 | start(2B) | quantity(2B) | byte_count | data(2N) | CRC | -> 请求
// | addr | 10 | start(2B) | quantity(2B) | CRC | -> 响应
#define MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS 0x10 // 写多个寄存器

// Modbus 异常码
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION     0x01
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS 0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE   0x03
#define MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE 0x04
#define MODBUS_EXCEPTION_ACKNOWLEDGE          0x05
#define MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY    0x06
#define MODBUS_EXCEPTION_MEMORY_PARITY_ERROR  0x08

#define MODBUS_TIMEOUT_MS                     (50)

typedef enum {
    MODBUS_STATE_IDLE                         = 0, // 空闲状态
    MODBUS_STATE_ADDRESS_DONE__FUNCTION_START = 1, // 已接收地址
    MODBUS_STATE_FUNCTION_DONE__DATA_START    = 2, // 已接收功能码
    MODBUS_STATE_DATA_DONE__CRC_LOW           = 3, // 已接收数据
    MODBUS_STATE_CRC_LOW_DONE__CRC_HIGH       = 4  // 已接收CRC低字节
} modbus_state_t;

typedef struct
{
    modbus_state_t state;          // 当前状态
    uint8_t address;               // 接收的地址
    uint8_t function;              // 接收的功能码
    uint8_t data[128];             // 接收的数据，最大长度为128字节
    uint16_t data_length;          // 接收的数据长度
    uint16_t expected_data_length; // 预期的数据长度，根据功能码确定
    uint16_t crc;                  // 接收的CRC校验值
    uint16_t calculated_crc;       // 计算的CRC校验值
    uint32_t last_char_time;       // 上次接收字符的时间戳，用于判断是否超时
} modbus_parser_t;

typedef enum {
    MODBUS_CMD_NONE                    = 0, // 无任务
    MODBUS_CMD_CLEAR_TOTALIZER         = 1, // 清空累计流量
    MODBUS_CMD_ZERO_LEARN_START        = 2, // 开始0漂学习
    MODBUS_CMD_SAVE_PARAMETERS         = 3, // 保存参数
    MODBUS_CMD_LOAD_DEFAULT_PARAMETERS = 4, // 回复默认参数
    MODBUS_CMD_CLEAR_ALARM             = 5, // 清除报警
    MODBUS_CMD_SOFT_RESET              = 6  // 软件复位
} modbus_cmd_t;

#endif
