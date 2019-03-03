#ifndef MAIN_QTK_TIMER_H_
#define MAIN_QTK_TIMER_H_

#include <stdint.h>

int qtk_timeout(uint32_t handle, int time, int session);
void qtk_updatetime(void);
uint32_t qtk_starttime(void);
uint64_t qtk_thread_time(void);	// for profile, in micro second
void qtk_timer_init(void);
void qtk_timer_clean(void);
uint64_t qtk_timer_now(void);
uint64_t qtk_timer_starttime(void);

#endif
