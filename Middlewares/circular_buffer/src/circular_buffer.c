#include "circular_buffer.h"
#include "elog.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

circular_buf_t *g_modbus_rx_cb = NULL;

void create_empty_circular_buffer(circular_buf_t *p_buffer)
{
    p_buffer = (circular_buf_t *)malloc(sizeof(circular_buf_t));
    if (p_buffer != NULL)
    {
        memset(p_buffer, 0, sizeof(circular_buf_t));
    }
    else
    {
        log_e("Failed to allocate memory for circular buffer");
    }
    return;
}

// uint8_t buffer_is_null(circular_buf_t *p_buffer)
// {
//     return (p_buffer == NULL) ? 0x01 : 0x00;
// }

// uint8_t buffer_is_empty(circular_buf_t *p_buffer)
// {
//     if (buffer_is_null(p_buffer))
//     {
//         return 0xFF;
//     }
//     // 如果head等于tail，说明缓冲区没有数据，即为空，也就是输出0x01；否则输出0x00
//     return (p_buffer->head == p_buffer->tail) ? 0x01 : 0x00;
// }

// uint8_t buffer_is_full(circular_buf_t *p_buffer)
// {
//     if (buffer_is_null(p_buffer))
//     {
//         return 0xFF;
//     }
//     // 如果(head + 1) % CIRCULAR_BUF_SIZE等于tail，说明缓冲区已满，输出0x01；否则输出0x00
//     return ((p_buffer->head + 1) % CIRCULAR_BUF_SIZE == p_buffer->tail) ? 0x01 : 0x00;
// }

// uint16_t buffer_get_count(circular_buf_t *p_buffer)
// {
//     if (buffer_is_null(p_buffer))
//     {
//         return 0xFFFF; // 返回0xFFFF表示错误
//     }

//     if (p_buffer->head >= p_buffer->tail)
//     {
//         return p_buffer->head - p_buffer->tail;
//     }
//     else
//     {
//         return CIRCULAR_BUF_SIZE - (p_buffer->tail - p_buffer->head);
//     }
// }

// uint8_t buffer_insert_data(circular_buf_t *p_buffer, uint8_t data)
// {
//     if (buffer_is_null(p_buffer))
//         return 0xFF;

//     if (buffer_is_full(p_buffer))
//         return 0xFE;

//     p_buffer->buffer[p_buffer->head] = data;

//     p_buffer->head = (p_buffer->head + 1) % CIRCULAR_BUF_SIZE;

//     return 0x00;
// }

// uint8_t buffer_get_data(circular_buf_t *p_buffer, uint8_t *data)
// {
//     if (buffer_is_null(p_buffer))
//         return 0xFF;

//     if (buffer_is_empty(p_buffer))
//         return 0xFE;

//     *data = p_buffer->buffer[p_buffer->tail];

//     p_buffer->tail = (p_buffer->tail + 1) % CIRCULAR_BUF_SIZE;

//     return 0x00;
// }

// uint8_t buffer_clear(circular_buf_t *p_buffer)
// {
//     if (buffer_is_null(p_buffer))
//         return 0xFF;

//     p_buffer->head = 0;
//     p_buffer->tail = 0;
//     return 0x00;
// }

// uint8_t buffer_peek(circular_buf_t *p_buffer, uint8_t *data)
// {
//     if (buffer_is_null(p_buffer))
//         return 0xFF;

//     if (buffer_is_empty(p_buffer))
//         return 0xFE;

//     *data = p_buffer->buffer[p_buffer->tail];

//     return 0x00;
// }

// uint8_t buffer_change_head(circular_buf_t *p_buffer, uint16_t new_head)
// {
//     if (buffer_is_null(p_buffer))
//         return 0xFF;
//     if (new_head >= CIRCULAR_BUF_SIZE)
//         return 0xFE;
//     p_buffer->head = new_head;
//     return 0x00;
// }