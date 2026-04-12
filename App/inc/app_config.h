#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/************************* software info define *************************/
#ifndef APP_SW_NAME
#define APP_SW_NAME "RUF-Q20"
#endif

#ifndef APP_SW_VERSION
#define APP_SW_VERSION "RUF-V1.0.2"
#endif

/************************* elog define *************************/
#ifndef APP_ELOG_ENABLE
#define APP_ELOG_ENABLE 1
#endif

#ifndef APP_ELOG_FILTER_LVL
#define APP_ELOG_FILTER_LVL 3
#endif

/************************* feature switch define *************************/
#ifndef USE_MODBUS
#define USE_MODBUS 1
#endif

#if defined(CCT6)
#define BOARD_HAS_E2PROM 1
#elif defined(RCT6)
#define BOARD_HAS_E2PROM 0
#else
#define BOARD_HAS_E2PROM 1
#endif

#ifndef USE_E2PROM
#define USE_E2PROM BOARD_HAS_E2PROM
#endif

#ifndef APP_ENABLE_MODBUS_RUNTIME
#define APP_ENABLE_MODBUS_RUNTIME 1
#endif

#ifndef APP_ENABLE_ELOG_TASK
#define APP_ENABLE_ELOG_TASK 1
#endif

#ifndef APP_ENABLE_ALGORITHM_TASK
#define APP_ENABLE_ALGORITHM_TASK 1
#endif

#ifndef APP_ENABLE_GUI_TASK
#define APP_ENABLE_GUI_TASK 1
#endif

#ifndef APP_ENABLE_KEY_TASK
#define APP_ENABLE_KEY_TASK APP_ENABLE_GUI_TASK
#endif

/************************* data source define *************************/
#define APP_MEASURE_SOURCE_FAKE_DATA 1U
#define APP_MEASURE_SOURCE_FPGA_SPI  2U

#ifndef APP_MEASURE_SOURCE
#define APP_MEASURE_SOURCE APP_MEASURE_SOURCE_FPGA_SPI
#endif

#define APP_ENABLE_MEASURE_FAKE_DATA ((APP_MEASURE_SOURCE) == APP_MEASURE_SOURCE_FAKE_DATA)
#define APP_ENABLE_MEASURE_FPGA_SPI  ((APP_MEASURE_SOURCE) == APP_MEASURE_SOURCE_FPGA_SPI)

/************************* fake data define *************************/
#define FAKE_DATA_MODE_SPEED 1
#define FAKE_DATA_MODE_FLOW  2

#ifndef FAKE_DATA_MODE
#define FAKE_DATA_MODE FAKE_DATA_MODE_SPEED
#endif

#ifndef APP_FAKE_DATA_LOG_PERIOD_MS
#define APP_FAKE_DATA_LOG_PERIOD_MS 5000U
#endif

/************************* spi rx debug define *************************/
#ifndef APP_SPI_RX_WAIT_TIMEOUT_MS
#define APP_SPI_RX_WAIT_TIMEOUT_MS 1000U
#endif

#ifndef APP_SPI_RX_READ_TIMEOUT_MS
#define APP_SPI_RX_READ_TIMEOUT_MS 20U
#endif

#ifndef APP_SPI_RX_INT_CLEAR_TIMEOUT_MS
#define APP_SPI_RX_INT_CLEAR_TIMEOUT_MS 20U
#endif

#ifndef APP_SPI_RX_VERBOSE_PACKETS
#define APP_SPI_RX_VERBOSE_PACKETS 1U
#endif

#ifndef APP_SPI_RX_PACKET_LOG_INTERVAL
#define APP_SPI_RX_PACKET_LOG_INTERVAL 125U
#endif

#ifndef APP_SPI_RX_ASSERTED_LOG_INTERVAL
#define APP_SPI_RX_ASSERTED_LOG_INTERVAL 125U
#endif

/************************* algorithm debug define *************************/
#ifndef APP_ALGO_OUTPUT_LOG_PERIOD_MS
#define APP_ALGO_OUTPUT_LOG_PERIOD_MS 2000U
#endif

/************************* legacy compatibility define *************************/
#ifndef ENABLE_FPGA_SPI_COMM_TEST
#define ENABLE_FPGA_SPI_COMM_TEST APP_ENABLE_MEASURE_FPGA_SPI
#endif

/************************* E2PROM define *************************/
#define E2PROM_AT24C08     0
#define E2PROM_AT24C32     1
#define USE_IIC_FOR_E2PROM 1

#endif /* APP_CONFIG_H */
