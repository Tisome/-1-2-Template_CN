// #ifndef __USART_H
// #define __USART_H

// #include "gd32f30x.h"
// #include <stdint.h>

// /* USART0 + DMA circular receive init */
// void usart0_dma_init(uint8_t *rxbuffer, uint32_t buflen);

// /* blocking transmit */
// void usart0_send_bytes(uint8_t *buf, uint32_t len);

// /* RS485/Modbus transmit */
// void usart0_send_modbus_bytes(uint8_t *buf, uint32_t len);

// #endif /* __USART_H */

#ifndef __USART_MODBUS_H__
#define __USART_MODBUS_H__

#include "gd32f30x.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================
 * 配置区
 * ========================= */

/* Modbus 接收缓冲区大小 */
#define MODBUS_RX_BUF_SIZE 128U

/* Modbus 波特率 */
#define MODBUS_BAUDRATE 9600U

extern volatile uint8_t s_modbus_rx_buf[MODBUS_RX_BUF_SIZE];
extern volatile uint16_t s_modbus_rx_len;
extern volatile uint8_t s_modbus_frame_ready;
extern volatile uint8_t s_modbus_rx_overflow;

/* =========================
 * 对外接口
 * ========================= */

/* USART0 初始化（不使用DMA） */
void usart0_modbus_init(void);

/* Modbus RTU 超时定时器初始化 */
void modbus_timer_init(void);

/* 普通阻塞发送 */
void usart0_send_bytes(uint8_t *buf, uint32_t len);

/* RS485 / Modbus 发送 */
void usart0_send_modbus_bytes(uint8_t *buf, uint32_t len);

/* 查询是否收到完整一帧 */
uint8_t modbus_frame_is_ready(void);

/* 取出一帧数据，返回长度 */
uint16_t modbus_get_frame(uint8_t *buf, uint16_t max_len);

/* 清空接收状态 */
void modbus_rx_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_MODBUS_H__ */