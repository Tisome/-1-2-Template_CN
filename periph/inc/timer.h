#ifndef __TIMER_H
#define __TIMER_H

#include "gd32f30x.h"

#define MODBUS_TIMER TIMER2

/* 定时器时钟假设为 120MHz | 预分频后变成 1MHz，即 1us 计一次 */
#define MODBUS_TIMER_PSC (120U - 1U)

/* 9600bps |  1byte time ≈ 10 / 9600 = 1.0417ms |  3.5T ≈ 3.646ms | as 4.5 ms */
#define MODBUS_FRAME_TIMEOUT_US 4500U
#define MODBUS_TIMER_PERIOD     (MODBUS_FRAME_TIMEOUT_US - 1U)

#define MODBUS_TIMER_IRQn       TIMER2_IRQn

void modbus_timer_init(void);

void modbus_timer_restart(void);

void modbus_timer_stop(void);

#endif