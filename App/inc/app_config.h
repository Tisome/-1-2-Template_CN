#define USE_MODBUS 1

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

/*
 * EasyLogger 运行时全局过滤级别。
 * 0~5 分别对应 ASSERT/ERROR/WARN/INFO/DEBUG/VERBOSE。
 * 关闭日志时该值不会生效。
 */
#ifndef APP_ELOG_FILTER_LVL
#define APP_ELOG_FILTER_LVL 3
#endif

/************************* E2PROM define *************************/
#if defined(CCT6)
#define BOARD_HAS_E2PROM 1
#elif defined(RCT6)
#define BOARD_HAS_E2PROM 0
#else
#define BOARD_HAS_E2PROM 1
#endif

#ifndef USE_E2PROM
#define USE_E2PROM BOARD_HAS_E2PROM

#define E2PROM_AT24C08     0
#define E2PROM_AT24C32     1

#define USE_IIC_FOR_E2PROM 1

#endif

/************************* fake data define *************************/
#define FAKE_DATA_MODE_SPEED 1
#define FAKE_DATA_MODE_FLOW  2

#ifndef FAKE_DATA_MODE
#define FAKE_DATA_MODE FAKE_DATA_MODE_SPEED
#endif
