#ifndef WTK_CORE_WTK_CALENDAR_H
#define WTK_CORE_WTK_CALENDAR_H
#include "wtk_type.h"
#ifdef __cplusplus
extern "C"{
#endif

typedef struct wtk_lunar{
	int year;
	int month;
	int day;
	unsigned is_leap;
} wtk_lunar_t;

typedef struct wtk_solar{
	int year;
	int month;
	int day;
} wtk_solar_t;

/**
 * 此农历和阳历之间的转换只支持从1901年到2099年
 */
int wtk_calendar_to_solar(wtk_lunar_t *lunar,wtk_solar_t *solar);
int wtk_calendar_to_lunar(wtk_solar_t *solar,wtk_lunar_t *lunar);
int wtk_calendar_is_leap_year(int year);

#ifdef __cplusplus
};
#endif
#endif

