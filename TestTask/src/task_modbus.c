#include "elog.h"

#include "does_it_work.h"

#include "FreeRTOS.h"
#include "task.h"

#include "modbus_frame_process.h"
#include "modbus_parse.h"

#include "usart.h"

#include <stdint.h>
#include <string.h>

/* =========================
 * 测试阶段开关
 * ========================= */
#define MODBUS_TEST_LINK_LAYER 1
#define MODBUS_TEST_RX_FRAME 0
#define MODBUS_TEST_PARSER 0
#define MODBUS_TEST_EXECUTE 0
#define MODBUS_TEST_RESPONSE 0

/* =========================
 * 测试用静态数据
 * ========================= */

/* 例子：读保持寄存器请求
 * slave = 0x01
 * func  = 0x03
 * start = 0x0000
 * qty   = 0x0002
 * crc   = 0x0BC4 (低字节在前 -> C4 0B)
 */
static const uint8_t g_test_req_read_hold_regs[] =
    {
        0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};

static void modbus_test_link_layer(void)
{
    log_i("modbus test: link layer start");

    /* send data to PC */
    const uint8_t tx_data[] = {0x55, 0xAA, 0x01, 0x02, 0x03, 0x04};
    usart0_send_modbus_bytes(tx_data, 6);

    log_i("modbus test: link layer send %u bytes", (uint32_t)sizeof(tx_data));
    log_i("modbus test: link layer done");
}

static void modbus_test_rx_frame(void)
{
    log_i("modbus test: rx frame start");

    /* 这里测试 DMA + IDLE 分帧结果
     * 一般这里不手写协议解析，只打印收到的一整帧
     * 你可以把队列里收到的数据 dump 出来
     */

    /* 伪代码：
     * if (xQueueReceive(queue_uart_frame, &frame, portMAX_DELAY) == pdPASS)
     * {
     *     log_i("rx len = %u", frame.len);
     *     dump_hex(frame.buf, frame.len);
     * }
     */

    log_i("modbus test: rx frame done");
}

static void modbus_test_parser(void)
{
    log_i("modbus test: parser start");

    modbus_parser_t parser;
    memset(&parser, 0, sizeof(parser));

    /* 按你自己的解析接口替换 */
    /* int ret = modbus_parse_frame(&parser,
                                 g_test_req_read_hold_regs,
                                 sizeof(g_test_req_read_hold_regs)); */

    /* 这里只写结构思路 */
    int ret = 0;

    if (ret != 0)
    {
        log_e("modbus parser failed, ret=%d", ret);
        return;
    }

    /* 假设 parser 里已经有这些字段 */
    log_i("parser ok");
    /* log_i("addr=%u func=0x%02X start=%u qty=%u",
           parser.slave_addr,
           parser.function_code,
           parser.start_addr,
           parser.quantity); */

    log_i("modbus test: parser done");
}

static void modbus_test_execute(void)
{
    log_i("modbus test: execute start");

    modbus_parser_t parser;
    memset(&parser, 0, sizeof(parser));

    /* 1. 先伪造一个已解析好的请求 */
    /* parser.slave_addr    = 0x01; */
    /* parser.function_code = 0x03; */
    /* parser.start_addr    = 0x0000; */
    /* parser.quantity      = 0x0002; */

    /* 2. 执行功能码处理 */
    /* int ret = modbus_execute_request(&parser); */
    int ret = 0;

    if (ret != 0)
    {
        log_e("modbus execute failed, ret=%d", ret);
        return;
    }

    log_i("execute ok");
    log_i("modbus test: execute done");
}

static void modbus_test_response(void)
{
    log_i("modbus test: response start");

    modbus_parser_t parser;
    memset(&parser, 0, sizeof(parser));

    uint8_t tx_buf[64] = {0};
    uint16_t tx_len = 0;

    /* 1. 构造一个执行完成后的结果 */
    /* parser.slave_addr    = 0x01; */
    /* parser.function_code = 0x03; */
    /* parser.quantity      = 2; */
    /* parser.response_data[0] = 0x12; */
    /* parser.response_data[1] = 0x34; */
    /* parser.response_data[2] = 0x56; */
    /* parser.response_data[3] = 0x78; */

    /* 2. 调响应帧打包接口 */
    /* int ret = modbus_build_response(&parser, tx_buf, sizeof(tx_buf), &tx_len); */
    int ret = 0;

    if (ret != 0)
    {
        log_e("modbus build response failed, ret=%d", ret);
        return;
    }

    log_i("response build ok, tx_len=%u", tx_len);

    /* 3. 真发给PC */
    /* rs485_send_bytes(tx_buf, tx_len); */

    log_i("modbus test: response done");
}

void task_modbus(void *p)
{
    (void)p;

    log_i("task_modbus start");

    while (1)
    {
#if MODBUS_TEST_LINK_LAYER
        modbus_test_link_layer();
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif

#if MODBUS_TEST_RX_FRAME
        modbus_test_rx_frame();
        vTaskDelay(pdMS_TO_TICKS(100));
#endif

#if MODBUS_TEST_PARSER
        modbus_test_parser();
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif

#if MODBUS_TEST_EXECUTE
        modbus_test_execute();
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif

#if MODBUS_TEST_RESPONSE
        modbus_test_response();
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif

#if (!MODBUS_TEST_LINK_LAYER) && \
    (!MODBUS_TEST_RX_FRAME) &&   \
    (!MODBUS_TEST_PARSER) &&     \
    (!MODBUS_TEST_EXECUTE) &&    \
    (!MODBUS_TEST_RESPONSE)
        log_i("task_modbus idle: no test enabled");
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif
    }
}