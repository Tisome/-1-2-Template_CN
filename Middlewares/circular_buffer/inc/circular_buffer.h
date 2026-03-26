#ifndef __CIRCULAR_BUF_H
#define __CIRCULAR_BUF_H

#include <stdint.h>

#define CIRCULAR_BUF_SIZE 128U

typedef struct
{
    uint16_t start;
    uint16_t end;
} modbus_frame_desc_t;

#define MODBUS_FRAME_FIFO_SIZE 8U

typedef struct
{
    uint8_t buffer[CIRCULAR_BUF_SIZE];

    volatile uint16_t read_pos;

    /* 帧描述FIFO */
    modbus_frame_desc_t frame_fifo[MODBUS_FRAME_FIFO_SIZE];
    volatile uint8_t frame_w;
    volatile uint8_t frame_r;
    volatile uint8_t frame_count;

} circular_buf_t;

// typedef struct {
//     uint8_t buffer[CIRCULAR_BUF_SIZE];
//     volatile uint16_t read_pos;    // parse任务读到哪里
//     volatile uint16_t frame_start; // 当前帧起点
//     volatile uint16_t frame_end;   // 当前帧结束
//     volatile uint16_t frame_ready; // 有一帧待解析
// } circular_buf_t;

circular_buf_t *modbus_buf_init(void);

circular_buf_t *create_empty_circular_buffer(void);

extern circular_buf_t *g_modbus_rx_cb;

// /**
//  * @brief Check if the circular buffer pointer is NULL
//  *
//  * @param p_buffer Pointer to the circular buffer
//  * @return uint8_t :
//  * 0x00: p_buffer is not NULL
//  * 0x01: p_buffer is NULL
//  */
// uint8_t buffer_is_null(circular_buf_t *p_buffer);

// /**
//  * @brief Check if the circular buffer is empty
//  *
//  * @param p_buffer Pointer to the circular buffer
//  * @return uint8_t :
//  * 0x00: p_buffer is not empty
//  * 0x01: p_buffer is empty
//  * 0xFF: p_buffer is NULL/ERROR
//  */
// uint8_t buffer_is_empty(circular_buf_t *p_buffer);

// /**
//  * @brief Check if the circular buffer is full
//  *
//  * @param p_buffer Pointer to the circular buffer
//  * @return uint8_t :
//  * 0x00: p_buffer is not full
//  * 0x01: p_buffer is full
//  * 0xFF: p_buffer is NULL/ERROR
//  */
// uint8_t buffer_is_full(circular_buf_t *p_buffer);

// /**
//  * @brief Get the number of elements in the circular buffer
//  *
//  * @param p_buffer Pointer to the circular buffer
//  * @return uint16_t : number of elements in the buffer
//  */
// uint16_t buffer_get_count(circular_buf_t *p_buffer);

// /**
//  * @brief Insert data into the circular buffer
//  *
//  * @param p_buffer Pointer to the circular buffer
//  * @param data Data to be inserted
//  * @return uint8_t :
//  * 0x00: Success
//  * 0xFF: p_buffer is NULL/ERROR
//  * 0xFE: Buffer is full
//  */
// uint8_t buffer_insert_data(circular_buf_t *p_buffer, uint8_t data);

// /**
//  * @brief Get data from the circular buffer
//  *
//  * @param p_buffer Pointer to the circular buffer
//  * @param data Pointer to store the retrieved data
//  * @return uint8_t :
//  * 0x00: Success
//  * 0xFF: p_buffer is NULL/ERROR
//  * 0xFE: Buffer is empty
//  */
// uint8_t buffer_get_data(circular_buf_t *p_buffer, uint8_t *data);

// /**
//  * @brief Clear the circular buffer
//  *
//  * @param p_buffer Pointer to the circular buffer
//  * @return uint8_t :
//  * 0x00: Success
//  * 0xFF: p_buffer is NULL/ERROR
//  */
// uint8_t buffer_clear(circular_buf_t *p_buffer);

// /**
//  * @brief Peek at the data at the front of the circular buffer
//  *
//  * @param p_buffer Pointer to the circular buffer
//  * @param data Pointer to store the peeked data
//  * @return uint8_t :
//  * 0x00: Success
//  * 0xFF: p_buffer is NULL/ERROR
//  * 0xFE: Buffer is empty
//  */
// uint8_t buffer_peek(circular_buf_t *p_buffer, uint8_t *data);

// /**
//  * @brief Change the head position of the circular buffer
//  *
//  * @param p_buffer Pointer to the circular buffer
//  * @param new_head New head position (0 to CIRCULAR_BUF_SIZE - 1)
//  * @return uint8_t :
//  * 0x00: Success
//  * 0xFF: p_buffer is NULL/ERROR
//  * 0xFE: new_head is out of bounds
//  */
// uint8_t buffer_change_head(circular_buf_t *p_buffer, uint16_t new_head);
#endif /* __CIRCULAR BUF_H */