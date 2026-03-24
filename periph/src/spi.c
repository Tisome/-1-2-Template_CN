#include "spi.h"

#include "gd32f30x.h"

#include "gpio.h"

/* FPGA-SPI init */
void spi1_init()
{
    spi_parameter_struct spi_init_struct;

    spi_i2s_deinit(FPGA_SPI);

    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE; /* Mode 0 */
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_64;
    spi_init_struct.endian = SPI_ENDIAN_MSB;

    spi_init(FPGA_SPI, &spi_init_struct);

    spi_enable(FPGA_SPI);
}
