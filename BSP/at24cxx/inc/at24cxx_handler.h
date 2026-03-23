#ifndef __AT24CXX_HANDLER_H
#define __AT24CXX_HANDLER_H

#include "at24cxx_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

#define E2PROM_PIPE_PARA_START_ADDR 0x0000

typedef enum {
    E2PROM_OK              = 0,
    E2PROM_BUSY            = 1,
    E2PROM_TIMEOUT         = 2,
    E2PROM_ERROR           = 3,
    E2PROM_ERROR_RESOURCE  = 4,
    E2PROM_ERROR_PARAMETER = 5,
    E2PROM_ERROR_NO_MEMORY = 6,
    E2PROM_RESERVED        = 7
} e2prom_status_t;

typedef enum {
    E2PROM_READ  = 0,
    E2PROM_WRITE = 1
} e2prom_data_operation_t;

typedef struct
{
    uint16_t mem_addr;
    uint8_t *buf;
    uint16_t len;
    e2prom_data_operation_t event;
} e2prom_event_t;

typedef struct
{
    void (*os_delay_ms)(uint32_t const ms);
    e2prom_status_t (*os_queue_create)(uint32_t const item_num,
                                       uint32_t const item_size,
                                       void **const queue_handle);
    e2prom_status_t (*os_queue_give)(void *const queue_handle,
                                     void *const item,
                                     uint32_t timeout);
    e2prom_status_t (*os_queue_take)(void *queue_handle,
                                     void *buf,
                                     uint32_t timeout);
} e2prom_handler_os_interface_t;

typedef struct
{
    iic_driver_t *iic_driver_instance;
    at24cxx_dev_info_t *dev_info_instance;
    at24cxx_driver_t *at24cxx_driver_instance;
    e2prom_handler_os_interface_t *os_interface_instance;
} e2prom_input_arg_t;

typedef struct
{
    at24cxx_handler_t *at24cxx_handler_instance;
    e2prom_handler_os_interface_t *os_interface_instance;
    void *event_queue_handle;
} e2prom_handler_t;

/*
 * Readable aliases for the EEPROM service layer.
 * The original names are kept for compatibility, while these aliases make it
 * easier to distinguish the current sync path from the legacy async backup
 * path.
 */
typedef e2prom_status_t eeprom_status_t;
typedef e2prom_data_operation_t eeprom_operation_t;
typedef e2prom_event_t eeprom_async_request_t;
typedef e2prom_handler_os_interface_t eeprom_async_os_adapter_t;
typedef e2prom_input_arg_t eeprom_async_bootstrap_t;
typedef e2prom_handler_t eeprom_async_service_t;

at24cxx_status_t at24cxx_handler_inst(at24cxx_handler_t *instance);

e2prom_status_t e2prom_handler_inst(e2prom_handler_t *instance,
                                    e2prom_input_arg_t *arg);

/* Legacy async backup task/API: kept for rollback if sync access is removed. */
void task_e2prom_handler(void *arg);

e2prom_status_t e2prom_write_async(uint16_t mem_addr, uint8_t *buf, uint16_t len);
e2prom_status_t e2prom_read_async(uint16_t mem_addr, uint8_t *buf, uint16_t len);

/* Current simple sync access API used by parameter save/load. */
eeprom_status_t eeprom_write_sync(uint16_t mem_addr, const uint8_t *buf, uint16_t len);
eeprom_status_t eeprom_read_sync(uint16_t mem_addr, uint8_t *buf, uint16_t len);

/* Readable aliases for the legacy async backup API. */
eeprom_status_t eeprom_legacy_async_write(uint16_t mem_addr, const uint8_t *buf, uint16_t len);
eeprom_status_t eeprom_legacy_async_read(uint16_t mem_addr, uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
