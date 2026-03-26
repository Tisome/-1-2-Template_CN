#ifndef __MODBUS_PARSE_H
#define __MODBUS_PARSE_H

#include "circular_buffer.h"

#include "modbus_protocol.h"

#include <stdint.h>

uint16_t modbus_frame_is_ready(circular_buf_t *p_buffer);

uint16_t modbus_get_frame(circular_buf_t *p_buffer, uint8_t *buf, uint16_t max_len);

void reset_modbus_parser(modbus_parser_t *parser);

void task_modbus_parse(void *parameter);

#endif