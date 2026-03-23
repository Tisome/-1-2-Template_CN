#include "at24cxx_handler.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "elog.h"

#define E2PROM_EVENT_QUEUE_LEN 16U
#define E2PROM_MAX_DELAY       0xFFFFFFFFUL

/* Reserved legacy async service instance kept as a rollback path. */
static eeprom_async_service_t *g_legacy_async_eeprom_service = NULL;

static eeprom_status_t eeprom_status_from_at24(at24cxx_status_t status)
{
    switch (status)
    {
    case AT24CXX_OK:
        return E2PROM_OK;
    case AT24CXX_BUSY:
        return E2PROM_BUSY;
    case AT24CXX_TIMEOUT:
        return E2PROM_TIMEOUT;
    case AT24CXX_ERROR_RESOURCE:
        return E2PROM_ERROR_RESOURCE;
    case AT24CXX_ERROR_PARAMETER:
        return E2PROM_ERROR_PARAMETER;
    case AT24CXX_ERROR_NO_MEMORY:
        return E2PROM_ERROR_NO_MEMORY;
    default:
        return E2PROM_ERROR;
    }
}

static eeprom_status_t eeprom_access_sync_internal(eeprom_operation_t operation,
                                                   uint16_t mem_addr,
                                                   uint8_t *buf,
                                                   uint16_t len)
{
#if USE_E2PROM
    iic_driver_t i2c_driver;
    eeprom_chip_driver_t eeprom_chip_driver;
    eeprom_chip_config_t eeprom_chip_config;
    iic_status_t i2c_status;
    at24cxx_status_t chip_status;

    if ((buf == NULL) || (len == 0U))
    {
        return E2PROM_ERROR_PARAMETER;
    }

    iic_driver_init(&i2c_driver);
    at24cxx_driver_init(&eeprom_chip_driver);

    chip_status = eeprom_chip_driver.pf_init(&eeprom_chip_config);
    if (chip_status != AT24CXX_OK)
    {
        return eeprom_status_from_at24(chip_status);
    }

    i2c_status = i2c_driver.pf_iic_init(i2c_driver.hi2c,
                                        (void *)E2PROM_IIC_INSTANCE,
                                        100000U);
    if (i2c_status != I2C_OK)
    {
        return E2PROM_ERROR_RESOURCE;
    }

    if (operation == E2PROM_WRITE)
    {
        chip_status = eeprom_chip_driver.pf_write(&eeprom_chip_config,
                                                  &i2c_driver,
                                                  mem_addr,
                                                  buf,
                                                  len);
    }
    else
    {
        chip_status = eeprom_chip_driver.pf_read(&eeprom_chip_config,
                                                 &i2c_driver,
                                                 mem_addr,
                                                 buf,
                                                 len);
    }

    return eeprom_status_from_at24(chip_status);
#else
    (void)operation;
    (void)mem_addr;
    (void)buf;
    (void)len;
    return E2PROM_ERROR_RESOURCE;
#endif
}

at24cxx_status_t at24cxx_handler_inst(at24cxx_handler_t *instance)
{
    if ((instance == NULL) || (instance->p_iic_driver_instance == NULL) ||
        (instance->p_at24cxx_dev_info == NULL) ||
        (instance->p_at24cxx_driver_instance == NULL))
    {
        return AT24CXX_ERROR_RESOURCE;
    }

    return AT24CXX_OK;
}

e2prom_status_t e2prom_handler_inst(e2prom_handler_t *instance,
                                    e2prom_input_arg_t *arg)
{
    if ((instance == NULL) || (arg == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    if ((instance->at24cxx_handler_instance == NULL) ||
        (instance->os_interface_instance == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    if ((arg->at24cxx_driver_instance == NULL) || (arg->dev_info_instance == NULL) ||
        (arg->iic_driver_instance == NULL) || (arg->os_interface_instance == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    instance->at24cxx_handler_instance->p_iic_driver_instance = arg->iic_driver_instance;
    instance->at24cxx_handler_instance->p_at24cxx_dev_info = arg->dev_info_instance;
    instance->at24cxx_handler_instance->p_at24cxx_driver_instance = arg->at24cxx_driver_instance;
    instance->os_interface_instance = arg->os_interface_instance;

    if (instance->os_interface_instance->os_queue_create == NULL)
    {
        return E2PROM_ERROR_RESOURCE;
    }

    if (instance->event_queue_handle == NULL)
    {
        e2prom_status_t qret = instance->os_interface_instance->os_queue_create(
            E2PROM_EVENT_QUEUE_LEN,
            sizeof(e2prom_event_t *),
            &(instance->event_queue_handle));
        if (qret != E2PROM_OK)
        {
            return qret;
        }
    }

    g_legacy_async_eeprom_service = instance;

    return E2PROM_OK;
}

static eeprom_status_t eeprom_legacy_async_execute(const eeprom_async_request_t *request)
{
    eeprom_chip_link_t *chip_link = g_legacy_async_eeprom_service->at24cxx_handler_instance;
    eeprom_chip_driver_t *chip_driver = chip_link->p_at24cxx_driver_instance;
    iic_driver_t *i2c_driver = chip_link->p_iic_driver_instance;
    at24cxx_status_t chip_status = AT24CXX_ERROR;

    if ((request == NULL) || (chip_driver == NULL) || (i2c_driver == NULL) || (i2c_driver->hi2c == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    if (request->event == E2PROM_WRITE)
    {
        chip_status = chip_driver->pf_write(chip_link->p_at24cxx_dev_info,
                                            i2c_driver,
                                            request->mem_addr,
                                            request->buf,
                                            request->len);
    }
    else
    {
        chip_status = chip_driver->pf_read(chip_link->p_at24cxx_dev_info,
                                           i2c_driver,
                                           request->mem_addr,
                                           request->buf,
                                           request->len);
    }

    return eeprom_status_from_at24(chip_status);
}

void task_e2prom_handler(void *arg)
{
    eeprom_async_bootstrap_t *legacy_async_bootstrap = (eeprom_async_bootstrap_t *)arg;
    eeprom_async_request_t *request = NULL;

    static eeprom_async_service_t legacy_async_service;
    static eeprom_chip_link_t legacy_async_chip_link;

    legacy_async_service.at24cxx_handler_instance = &legacy_async_chip_link;
    legacy_async_service.os_interface_instance = legacy_async_bootstrap->os_interface_instance;
    legacy_async_service.event_queue_handle = NULL;

    if (e2prom_handler_inst(&legacy_async_service, legacy_async_bootstrap) != E2PROM_OK)
    {
        log_e("legacy async eeprom service init failed");
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        eeprom_status_t status = g_legacy_async_eeprom_service->os_interface_instance->os_queue_take(
            g_legacy_async_eeprom_service->event_queue_handle,
            &request,
            E2PROM_MAX_DELAY);
        if ((status != E2PROM_OK) || (request == NULL))
        {
            continue;
        }

        (void)eeprom_legacy_async_execute(request);
        vPortFree(request);
    }
}

eeprom_status_t eeprom_write_sync(uint16_t mem_addr, const uint8_t *buf, uint16_t len)
{
    return eeprom_access_sync_internal(E2PROM_WRITE, mem_addr, (uint8_t *)buf, len);
}

eeprom_status_t eeprom_read_sync(uint16_t mem_addr, uint8_t *buf, uint16_t len)
{
    return eeprom_access_sync_internal(E2PROM_READ, mem_addr, buf, len);
}

e2prom_status_t e2prom_write_async(uint16_t mem_addr, uint8_t *buf, uint16_t len)
{
    if ((g_legacy_async_eeprom_service == NULL) ||
        (g_legacy_async_eeprom_service->event_queue_handle == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    eeprom_async_request_t *request = pvPortMalloc(sizeof(eeprom_async_request_t));
    if (request == NULL)
    {
        return E2PROM_ERROR_NO_MEMORY;
    }

    request->mem_addr = mem_addr;
    request->buf = buf;
    request->len = len;
    request->event = E2PROM_WRITE;

    eeprom_status_t status = g_legacy_async_eeprom_service->os_interface_instance->os_queue_give(
        g_legacy_async_eeprom_service->event_queue_handle,
        &request,
        E2PROM_MAX_DELAY);
    if (status != E2PROM_OK)
    {
        vPortFree(request);
    }

    return status;
}

e2prom_status_t e2prom_read_async(uint16_t mem_addr, uint8_t *buf, uint16_t len)
{
    if ((g_legacy_async_eeprom_service == NULL) ||
        (g_legacy_async_eeprom_service->event_queue_handle == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    eeprom_async_request_t *request = pvPortMalloc(sizeof(eeprom_async_request_t));
    if (request == NULL)
    {
        return E2PROM_ERROR_NO_MEMORY;
    }

    request->mem_addr = mem_addr;
    request->buf = buf;
    request->len = len;
    request->event = E2PROM_READ;

    eeprom_status_t status = g_legacy_async_eeprom_service->os_interface_instance->os_queue_give(
        g_legacy_async_eeprom_service->event_queue_handle,
        &request,
        E2PROM_MAX_DELAY);
    if (status != E2PROM_OK)
    {
        vPortFree(request);
    }

    return status;
}

eeprom_status_t eeprom_legacy_async_write(uint16_t mem_addr, const uint8_t *buf, uint16_t len)
{
    return e2prom_write_async(mem_addr, (uint8_t *)buf, len);
}

eeprom_status_t eeprom_legacy_async_read(uint16_t mem_addr, uint8_t *buf, uint16_t len)
{
    return e2prom_read_async(mem_addr, buf, len);
}
