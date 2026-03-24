#include "modbus_parse.h"
#include "modbus_crc.h"
#include "modbus_frame_process.h"
#include "modbus_protocol.h"

#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#include "elog.h"

// 使用 memset 将 parser 结构体的所有成员初始化为 0
// 同时设置状态为IDLE
void reset_modbus_parser(modbus_parser_t *parser)
{
    memset(parser, 0, sizeof(modbus_parser_t));
    parser->state = MODBUS_STATE_IDLE;
}

void task_modbus_parse(void *parameter)
{

    uint8_t byte = 0U;
    uint8_t buf[MODBUS_RX_BUF_SIZE];
    uint16_t raw_len = 0U;
    modbus_parser_t parser;
    reset_modbus_parser(&parser);

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (0U == modbus_frame_is_ready())
        {
            log_e("modbus frame is not ready");
            continue;
        }

        reset_modbus_parser(&parser);

        raw_len = modbus_get_frame(buf, MODBUS_RX_BUF_SIZE);

        // 没有接收到完整的modbus帧

        for (int i = 0; i < raw_len; i++)
        {
            byte = buf[i];
            // 根据 parser.state 的状态来解析 Modbus 数据
            switch (parser.state)
            {
            case MODBUS_STATE_IDLE:
                // 处理空闲状态，等待地址字节
                if (byte != MODBUS_SLAVE_ADDR)
                {
                    log_e("Received byte does not match Modbus slave address, ignoring");
                    break; // 继续等待下一个字节
                }
                else
                {
                    parser.state = MODBUS_STATE_ADDRESS_DONE__FUNCTION_START;
                    parser.address = byte;
                    parser.calculated_crc = 0xFFFF;
                    parser.calculated_crc = modbus_crc_update(parser.calculated_crc, byte);
                }
            case MODBUS_STATE_ADDRESS_DONE__FUNCTION_START:
                // 地址字节已经被处理，现在在处理功能码
                parser.function = byte;
                parser.calculated_crc = modbus_crc_update(parser.calculated_crc, byte);
                switch (parser.function)
                {
                // 这些功能码后面跟四字节
                // 起始地址2字节 + 数量2字节
                case (MODBUS_FUNC_READ_COILS):
                case (MODBUS_FUNC_READ_DISCRETE_INPUTS):
                case (MODBUS_FUNC_READ_HOLDING_REGISTERS):
                case (MODBUS_FUNC_READ_INPUT_REGISTERS):
                    parser.expected_data_length = 4;
                    break;

                // 这些功能码后面跟四字节
                // 起始地址2字节 + 值2字节
                case (MODBUS_FUNC_WRITE_SINGLE_COIL):
                case (MODBUS_FUNC_WRITE_SINGLE_REGISTER):
                    parser.expected_data_length = 4;
                    break;

                // 该功能码后面的格式是
                // 起始地址2字节 + 寄存器数量2字节 + 要写入的字节数1字节 + N字节个数据
                case (MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS):
                    parser.expected_data_length = 5;
                    break;

                default:
                    parser.expected_data_length = 0;
                    break;
                }
                parser.data_length = 0;
                parser.state = MODBUS_STATE_FUNCTION_DONE__DATA_START;
                break;
            case MODBUS_STATE_FUNCTION_DONE__DATA_START:
                // 功能码字节已经被处理，现在在处理数据段
                if (parser.data_length >= sizeof(parser.data))
                {
                    log_e("Modbus data buffer overflow");
                    reset_modbus_parser(&parser);
                    break;
                }
                parser.data[parser.data_length++] = byte;
                parser.calculated_crc = modbus_crc_update(parser.calculated_crc, byte);

                // 判断多数据写入的情况
                if (parser.function == MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS &&
                    parser.data_length == 5)
                {
                    // 在功能码之后的数据结构是
                    // 起始地址2字节 + 寄存器数量2字节 + 要写入的字节数1字节
                    // 因此取出data[4]就是剩下的要写入的字节
                    // 这时候要增加expected_data_length的长度
                    uint8_t byte_cnt = parser.data[4];
                    parser.expected_data_length = byte_cnt + 5;
                }

                if (parser.data_length == parser.expected_data_length)
                {
                    parser.state = MODBUS_STATE_DATA_DONE__CRC_LOW;
                }

                break;
            case MODBUS_STATE_DATA_DONE__CRC_LOW:
                // 数据字节处理结束，开始接收CRC低字节
                parser.crc = (uint16_t)byte;
                parser.state = MODBUS_STATE_CRC_LOW_DONE__CRC_HIGH;
                break;
            case MODBUS_STATE_CRC_LOW_DONE__CRC_HIGH:
                // 处理CRC高字节
                parser.crc |= (byte << 8);

                if (parser.calculated_crc == parser.crc)
                {
                    process_modbus_frame(&parser);
                }
                else
                {
                    log_e("Error modbus crc");
                }
                reset_modbus_parser(&parser);

                break;
            default:
                // 处理未知状态
                parser.state = MODBUS_STATE_IDLE;
                break;
            }
        }
    }
}