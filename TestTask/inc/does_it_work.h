#ifndef DOES_IT_WORK_H
#define DOES_IT_WORK_H

#ifdef __cplusplus
extern "C" {
#endif

void task_clock(void *p);
void task_elog(void *p);
void task_e2prom(void *p);

void does_it_work(void);

#ifdef __cplusplus
}
#endif

#endif /* DOES_IT_WORK_H */