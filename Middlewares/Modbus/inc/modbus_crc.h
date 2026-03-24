#ifndef __MODBUS_CRC_H
#define __MODBUS_CRC_H

#include <stdint.h>

uint16_t modbus_crc_update(uint16_t crc, uint8_t data);

#endif