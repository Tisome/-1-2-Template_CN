#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>

// TFT - SPI2
// FPGA - SPI1

#define FPGA_SPI SPI1

typedef enum {
    SPI_STATUS_OK = 0,
    SPI_STATUS_TIMEOUT,
    SPI_STATUS_ERROR_PARAMETER
} spi_status_t;

void spi1_init(void);

spi_status_t spi_master_read_packet_timeout(uint32_t spi_periph,
                                            uint8_t *rx_buf,
                                            uint16_t len,
                                            uint32_t timeout_ms);

#endif