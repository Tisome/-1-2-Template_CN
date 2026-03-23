#ifndef ALGORITHM_PACKET_H
#define ALGORITHM_PACKET_H

#include <stdbool.h>
#include <stdint.h>
#include "data.h"

void rufx_unpack_packet(const rufx_raw_packet_t *raw,
                        rufx_packet_t *out,
                        uint8_t *seq);

bool rufx_calc_t1_t2_dt(const rufx_packet_t *pkt,
                        double *t1,
                        double *t2,
                        double *dt);

#endif