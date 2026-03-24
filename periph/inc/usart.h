
#ifndef __USART_MODBUS_H__
#define __USART_MODBUS_H__

#include "gd32f30x.h"

#include "circular_buffer.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_RX_BUF_SIZE CIRCULAR_BUF_SIZE

uint16_t usart0_dma_get_pos(void);

void usart0_dma_modbus_init(uint8_t *modbus_rx_dma_buf, uint16_t buf_size);

/* RS485 / Modbus 发送 */
void usart0_send_modbus_bytes(uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __USART_MODBUS_H__ */