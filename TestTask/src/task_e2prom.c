#include "at24cxx_handler.h"
#include "elog.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>
#include <string.h>

#define E2PROM_TEST_ADDR 0x0040U
#define E2PROM_TEST_LEN 16U

static void e2prom_rw_test_once(void)
{
    uint8_t write_buf[E2PROM_TEST_LEN] = {0};
    uint8_t read_buf[E2PROM_TEST_LEN] = {0};

    for (uint32_t i = 0; i < E2PROM_TEST_LEN; i++)
    {
        write_buf[i] = (uint8_t)(0xA0U + i);
    }

    eeprom_status_t write_status = eeprom_write_sync(E2PROM_TEST_ADDR,
                                                     write_buf,
                                                     E2PROM_TEST_LEN);
    if (write_status != E2PROM_OK)
    {
        log_e("e2prom write failed, status=%d", write_status);
        return;
    }

    /* 等 EEPROM 内部写周期完成 */
    vTaskDelay(pdMS_TO_TICKS(10));

    eeprom_status_t read_status = eeprom_read_sync(E2PROM_TEST_ADDR,
                                                   read_buf,
                                                   E2PROM_TEST_LEN);
    if (read_status != E2PROM_OK)
    {
        log_e("e2prom read failed, status=%d", read_status);
        return;
    }

    if (memcmp(write_buf, read_buf, E2PROM_TEST_LEN) != 0)
    {
        log_e("e2prom data mismatch");

        for (uint32_t i = 0; i < E2PROM_TEST_LEN; i++)
        {
            log_i("idx=%lu write=0x%02X read=0x%02X",
                  i,
                  write_buf[i],
                  read_buf[i]);
        }
        return;
    }

    log_i("e2prom rw test pass, addr=0x%04X len=%u",
          E2PROM_TEST_ADDR,
          E2PROM_TEST_LEN);
}

void task_e2prom(void *p)
{
    (void)p;

    log_i("task_e2prom start");
    e2prom_rw_test_once();
    log_i("task_e2prom done");

    vTaskDelete(NULL);
}