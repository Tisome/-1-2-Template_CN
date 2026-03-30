#include "elog.h"

#include "does_it_work.h"

#include "FreeRTOS.h"
#include "task.h"

#include "modbus_crc.h"
#include "modbus_frame_process.h"
#include "modbus_parse.h"

#include "usart.h"

#include <stdint.h>
#include <string.h>

/* =========================
 * 测试阶段开�?
 * ========================= */
#define MODBUS_TEST_LINK_LAYER_SEND 0
#define MODBUS_TEST_LINK_LAYER_RECEIVE 0
#define MODBUS_TEST_RX_FRAME 0
#define MODBUS_TEST_PARSER 1
#define MODBUS_TEST_EXECUTE 0
#define MODBUS_TEST_RESPONSE 0

#define MODBUS_TEST_SLAVE_ADDR 0x01

/* =========================
 * 测试用静态数�?
 * ========================= */

/* 例子：读保持寄存器请�?
 * slave = 0x01
 * func  = 0x03
 * start = 0x0000
 * qty   = 0x0002
 * crc   = 0x0BC4 (低字节在�?-> C4 0B)
 */
static const uint8_t g_test_req_read_hold_regs[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};

#if MODBUS_TEST_LINK_LAYER_SEND
static void modbus_test_link_layer(void)
{
    log_i("modbus test: link layer start");

    /* send data to PC */
    const uint8_t tx_data[] = {0x55, 0xAA, 0x01, 0x02, 0x03, 0x04};
    usart0_send_modbus_bytes(tx_data, 6);

    log_i("modbus test: link layer send %u bytes", (uint32_t)sizeof(tx_data));
    log_i("modbus test: link layer done");
}
#endif

#if MODBUS_TEST_LINK_LAYER_RECEIVE
static void modbus_test_link_layer_receive_callback(void)
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        log_i("receive a dma callback");
        usart0_send_modbus_bytes(g_test_req_read_hold_regs, 8U);
    }
}

#endif

#if MODBUS_TEST_RX_FRAME
static void modbus_test_rx_frame(void)
{
    log_i("modbus test: rx frame start");
    modbus_parser_t parser;

    uint16_t raw_len = 0U;
    uint8_t buf[MODBUS_RX_BUF_SIZE];

    /* 这里测试 DMA + IDLE 分帧结果
     * 一般这里不手写协议解析，只打印收到的一整帧
     * 把队列里收到的数�?dump 出来
     */
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        log_i("receive a dma callback");

        if (0U == modbus_frame_is_ready(g_modbus_rx_cb))
        {
            log_e("modbus frame is not ready");
            continue;
        }

        reset_modbus_parser(&parser);

        raw_len = modbus_get_frame(g_modbus_rx_cb, buf, MODBUS_RX_BUF_SIZE);
        usart0_send_modbus_bytes(buf, raw_len);
    }
}
#endif

#if MODBUS_TEST_PARSER

static void test_process_modbus_frame(modbus_parser_t *parser)
{
    if (parser->address != MODBUS_TEST_SLAVE_ADDR)
    {
        log_e("Modbus address mismatch");
        return;
    }

    switch (parser->function)
    {
    case MODBUS_FUNC_READ_COILS:
        log_i("MODBUS_FUNC_READ_COILS");
        break;

    case MODBUS_FUNC_READ_DISCRETE_INPUTS:
        log_i("MODBUS_FUNC_READ_DISCRETE_INPUTS");
        break;

    case MODBUS_FUNC_READ_HOLDING_REGISTERS:
        log_i("MODBUS_FUNC_READ_HOLDING_REGISTERS");
        break;

    case MODBUS_FUNC_READ_INPUT_REGISTERS:
        log_i("MODBUS_FUNC_READ_INPUT_REGISTERS");
        break;

    case MODBUS_FUNC_WRITE_SINGLE_COIL:
        log_i("MODBUS_FUNC_WRITE_SINGLE_COIL");
        break;

    case MODBUS_FUNC_WRITE_SINGLE_REGISTER:
        log_i("MODBUS_FUNC_WRITE_SINGLE_REGISTER");
        break;

    case MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS:
        log_i("MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS");
        break;

    default:
        log_i("MODBUS_EXCEPTION_ILLEGAL_FUNCTION");
        break;
    }
}

static int modbus_parse_bytes(modbus_parser_t *parser, const uint8_t *buf, uint16_t raw_len)
{
    uint8_t byte = 0U;

    if ((parser == NULL) || (buf == NULL) || (raw_len == 0U))
    {
        log_e("modbus_parse_bytes param error");
        return -1;
    }

    reset_modbus_parser(parser);

    for (uint16_t i = 0; i < raw_len; i++)
    {
        byte = buf[i];

        switch (parser->state)
        {
        case MODBUS_STATE_IDLE:
            if (byte != MODBUS_TEST_SLAVE_ADDR)
            {
                log_e("addr mismatch: recv=0x%02X expect=0x%02X", byte, MODBUS_TEST_SLAVE_ADDR);
                break;
            }

            parser->state = MODBUS_STATE_ADDRESS_DONE__FUNCTION_START;
            parser->address = byte;
            parser->calculated_crc = 0xFFFF;
            parser->calculated_crc = modbus_crc_update(parser->calculated_crc, byte);

            log_i("address ok: 0x%02X", parser->address);
            break;

        case MODBUS_STATE_ADDRESS_DONE__FUNCTION_START:
            parser->function = byte;
            parser->calculated_crc = modbus_crc_update(parser->calculated_crc, byte);

            switch (parser->function)
            {
            case MODBUS_FUNC_READ_COILS:
            case MODBUS_FUNC_READ_DISCRETE_INPUTS:
            case MODBUS_FUNC_READ_HOLDING_REGISTERS:
            case MODBUS_FUNC_READ_INPUT_REGISTERS:
            case MODBUS_FUNC_WRITE_SINGLE_COIL:
            case MODBUS_FUNC_WRITE_SINGLE_REGISTER:
                parser->expected_data_length = 4;
                break;

            case MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS:
                parser->expected_data_length = 5;
                break;

            default:
                log_e("unsupported function: 0x%02X", parser->function);
                reset_modbus_parser(parser);
                return -2;
            }

            parser->data_length = 0;
            parser->state = MODBUS_STATE_FUNCTION_DONE__DATA_START;

            log_i("function ok: 0x%02X", parser->function);

            break;

        case MODBUS_STATE_FUNCTION_DONE__DATA_START:
            if (parser->data_length >= sizeof(parser->data))
            {
                log_e("Modbus data buffer overflow");
                reset_modbus_parser(parser);
                return -3;
            }

            parser->data[parser->data_length++] = byte;
            parser->calculated_crc = modbus_crc_update(parser->calculated_crc, byte);

            if ((parser->function == MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS) &&
                (parser->data_length == 5))
            {
                uint8_t byte_cnt = parser->data[4];
                parser->expected_data_length = (uint16_t)byte_cnt + 5U;
            }

            if (parser->data_length == parser->expected_data_length)
            {
                parser->state = MODBUS_STATE_DATA_DONE__CRC_LOW;
                log_i("data_length=%u expected=%u", parser->data_length, parser->expected_data_length);
            }
            break;

        case MODBUS_STATE_DATA_DONE__CRC_LOW:
            parser->crc = (uint16_t)byte;
            parser->state = MODBUS_STATE_CRC_LOW_DONE__CRC_HIGH;
            log_i("crc low=0x%02X", byte);
            break;

        case MODBUS_STATE_CRC_LOW_DONE__CRC_HIGH:
            parser->crc |= ((uint16_t)byte << 8);

            if (parser->calculated_crc == parser->crc)
            {
                log_i("modbus crc ok");
                log_i("crc high=0x%02X", byte);
                test_process_modbus_frame(parser);
                reset_modbus_parser(parser);
                return 0;
            }
            else
            {
                log_e("modbus crc error calc=0x%04X recv=0x%04X",
                      parser->calculated_crc, parser->crc);
                reset_modbus_parser(parser);
                return -4;
            }

        default:
            log_e("unknown parser state");
            reset_modbus_parser(parser);
            return -5;
        }
    }

    log_e("frame incomplete, raw_len=%u", raw_len);
    return -6;
}

static void modbus_test_parser(void)
{
    uint8_t buf[MODBUS_RX_BUF_SIZE];
    uint16_t raw_len = 0U;
    modbus_parser_t parser;
    int ret;

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        log_i("receive a dma callback");

        if (g_modbus_rx_cb == NULL)
        {
            log_e("g_modbus_rx_cb == NULL");
            return;
        }

        if (0U == modbus_frame_is_ready(g_modbus_rx_cb))
        {
            log_e("modbus frame is not ready");
            return;
        }

        raw_len = modbus_get_frame(g_modbus_rx_cb, buf, MODBUS_RX_BUF_SIZE);
        if (raw_len == 0U)
        {
            log_e("modbus_get_frame returned 0");
            return;
        }

        log_i("uart frame received, len=%u", raw_len);

        // for (uint16_t i = 0; i < raw_len; i++)
        // {
        //     log_i("buf[%u] = 0x%02X", i, buf[i]);
        // }

        ret = modbus_parse_bytes(&parser, buf, raw_len);
        if (ret != 0)
        {
            log_e("modbus parse failed, ret=%d", ret);
        }
        else
        {
            log_i("modbus parse success");
        }
    }
}
#endif

#if MODBUS_TEST_EXECUTE
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

    /* 2. 执行功能码处�?*/
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
#endif

#if MODBUS_TEST_RESPONSE
static void modbus_test_response(void)
{
    log_i("modbus test: response start");

    modbus_parser_t parser;
    memset(&parser, 0, sizeof(parser));

    uint8_t tx_buf[64] = {0};
    uint16_t tx_len = 0;

    /* 1. 构造一个执行完成后的结�?*/
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

#endif

void task_modbus(void *p)
{
    (void)p;
    (void)g_test_req_read_hold_regs;

    log_i("task_modbus start");

    while (1)
    {
#if MODBUS_TEST_LINK_LAYER_SEND
        modbus_test_link_layer();
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif

#if MODBUS_TEST_LINK_LAYER_RECEIVE
        modbus_test_link_layer_receive_callback();
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

#if (!MODBUS_TEST_LINK_LAYER_SEND) &&    \
    (!MODBUS_TEST_LINK_LAYER_RECEIVE) && \
    (!MODBUS_TEST_RX_FRAME) &&           \
    (!MODBUS_TEST_PARSER) &&             \
    (!MODBUS_TEST_EXECUTE) &&            \
    (!MODBUS_TEST_RESPONSE)
        log_i("task_modbus idle: no test enabled");
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif
    }
}

#if MODBUS_TEST

#if MODBUS_TEST_LINK_LAYER_RECEIVE
void USART0_IRQHandler(void)
{
    volatile uint32_t temp;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE))
    {
        // 清标�?
        temp = USART_STAT0(USART0);
        temp = USART_DATA(USART0);
        (void)temp;

        TaskHandle_t task_modbus_test_handle = get_modbus_task_handle();
        if (task_modbus_test_handle != NULL)
        {
            vTaskNotifyGiveFromISR(task_modbus_test_handle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
#endif

#if MODBUS_TEST_RX_FRAME
void USART0_IRQHandler(void)
{
    uint16_t dma_pos;

    volatile uint32_t temp;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE))
    {
        // 清标�?
        temp = USART_STAT0(USART0);
        temp = USART_DATA(USART0);
        (void)temp;

        // 获得dma当前写到的位�?
        dma_pos = usart0_dma_get_pos();

        if (g_modbus_rx_cb == NULL)
        {
            log_e("g_modbus_rx_cb == NULL");
            return;
        }

        g_modbus_rx_cb->frame_start = g_modbus_rx_cb->read_pos;
        g_modbus_rx_cb->frame_end = dma_pos;
        g_modbus_rx_cb->frame_ready = 1;

        TaskHandle_t task_modbus_test_handle = get_modbus_task_handle();
        if (task_modbus_test_handle != NULL)
        {
            vTaskNotifyGiveFromISR(task_modbus_test_handle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
#endif

#if MODBUS_TEST_PARSER
void USART0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (SET == usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE))
    {
        uint16_t dma_pos;
        uint16_t frame_start;

        /* 清IDLE标志 */
        usart0_read_byte();

        dma_pos = usart0_dma_get_pos();
        frame_start = g_modbus_rx_cb->read_pos;

        /* 把这一帧压入FIFO */
        TaskHandle_t task_modbus_test_handle = get_modbus_task_handle();
        if (task_modbus_test_handle != NULL)
        {
            if (modbus_push_frame_from_isr(g_modbus_rx_cb, frame_start, dma_pos) == 1U)
            {
                vTaskNotifyGiveFromISR(task_modbus_test_handle, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
            else
            {
                log_i("push modbus frame in isr fail");
            }
        }
    }
}

#endif

#endif