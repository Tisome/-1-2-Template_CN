#include "periph_link.h"
void hardware_periph_init()
{
    rcu_config();
    gpio_config();
}