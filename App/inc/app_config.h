#define USE_MODBUS 1

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
