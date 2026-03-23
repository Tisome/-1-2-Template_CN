#ifndef ALGORITHM_PROCESS_H
#define ALGORITHM_PROCESS_H

#include "data.h"

bool algorithm_process_group(Pipe_Parameters_t *para,
                             Pipe_algo_state_t *state,
                             Pipe_algo_out_data_t *out,
                             double t1_ns,
                             double t2_ns,
                             double dt_ns);

#endif