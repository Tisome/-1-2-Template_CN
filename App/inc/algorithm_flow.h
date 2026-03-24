#ifndef ALGORITHM_FLOW_H
#define ALGORITHM_FLOW_H

#include <stdbool.h>
#include "data.h"

double calc_t_wall_ns(const Pipe_Parameters_t *para);

double sq_get_percent(const Pipe_algo_state_t *s);
void sq_window_update(Pipe_algo_state_t *s, bool is_bad);

bool flow_window_add(Pipe_algo_state_t *state,
                     double *v_raw,
                     double *v_avg);

double run_kalman_filter(kalman_t *k, double measurement);

double vel_calc_from_dt(const Pipe_Parameters_t *para,
                        double t1_ns,
                        double t2_ns,
                        double dt_ns);

double flow_drift_comp(Pipe_Parameters_t *para,
                       Pipe_algo_state_t *state,
                       double v,
                       double sq);

bool zero_learn_manual_start(Pipe_Parameters_t *para,
                             Pipe_algo_state_t *state);

double flow_limit(Pipe_Parameters_t *para, double v);

void update_flow_outputs(Pipe_Parameters_t *para,
                         Pipe_algo_state_t *state,
                         Pipe_algo_out_data_t *out,
                         double sq,
                         double v_mps);

void flow_alarm(Pipe_Parameters_t *para,
                double flow_speed_mps);

#endif
