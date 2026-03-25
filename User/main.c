#include "main.h"

#include "elog.h"
#include "periph_link.h"

#include "does_it_work.h"



int main(void)
{
    hardware_periph_init();

    my_elog_init();

    does_it_work();
}
