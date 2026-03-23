#ifndef __MODBUS_FRAME_PROCESS_H__
#define __MODBUS_FRAME_PROCESS_H__

#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "modbus_protocol.h"

#define MODBUS_COIL_COUNT           16
#define MODBUS_DISCRETE_INPUT_COUNT 16
#define MODBUS_HOLDING_REG_COUNT    64
#define MODBUS_INPUT_REG_COUNT      64

void init_modbus_data(void);
void update_input_registers(void);
void update_holding_registers_from_parameters(void);

void process_modbus_frame(modbus_parser_t *parser);

extern uint8_t g_modbus_coils[(MODBUS_COIL_COUNT + 7) / 8];
extern uint8_t g_modbus_discrete_inputs[(MODBUS_DISCRETE_INPUT_COUNT + 7) / 8];
extern uint16_t g_modbus_holding_registers[MODBUS_HOLDING_REG_COUNT];
extern uint16_t g_modbus_input_registers[MODBUS_INPUT_REG_COUNT];

extern QueueHandle_t g_modbus_cmd_queue;

#endif
