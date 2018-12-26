#ifndef WTK_CORE_WTK_TIME_H_
#define WTK_CORE_WTK_TIME_H_
#include <time.h>
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif

int wtk_time_is_leap_year(int y);
/**
 *	m=[1,12]
 */
int wtk_time_month_max_day(int m,int y);
#ifdef __cplusplus
};
#endif
#endif
