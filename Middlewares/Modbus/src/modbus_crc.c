#include "modbus_crc.h"
#include <stdint.h>

uint16_t modbus_crc_update(uint16_t crc, uint8_t data)
{
    crc ^= (uint16_t)data; // 当前字节异或进去

    for (uint8_t i = 0; i < 8; i++)
    {
        if (crc & 0x0001)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc >>= 1;
    }

    return crc;
}