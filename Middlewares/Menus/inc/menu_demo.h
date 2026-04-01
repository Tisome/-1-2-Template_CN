#ifndef MENU_DEMO_H
#define MENU_DEMO_H

#include "menu.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const menu_t *menu_demo_get_root(void);
menu_key_t menu_demo_map_key(uint8_t key_value);

#ifdef __cplusplus
}
#endif

#endif /* MENU_DEMO_H */
