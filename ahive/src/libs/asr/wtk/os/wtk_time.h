#ifndef WTK_CORE_WTK_TIME_H_
#define WTK_CORE_WTK_TIME_H_
#include "wtk/core/wtk_type.h"
#include "wtk/os/wtk_lock.h"
#include "wtk/core/wtk_str.h"
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif
#define WTK_TIME_SLOTS 2
#define wtk_time_ms(t) ((t)->wtk_cached_time)
typedef struct wtk_time wtk_time_t;

struct wtk_time
{
	struct tm tm;
	double wtk_cached_time;			//ms
	//char* http_time;
	//char* log_time;
	char cached_http_time[WTK_TIME_SLOTS][sizeof("Mon, 28 Sep 1970 06:00:00 GMT          ")];
	char cached_log_time[WTK_TIME_SLOTS][sizeof("1970/09/28 12:00:00                     ")];
	uint8_t	slot;
	wtk_lock_t lock;
	wtk_string_t http_time;
	wtk_string_t log_time;
};

wtk_time_t* wtk_time_new();
int wtk_time_delete(wtk_time_t *t);
int wtk_time_init(wtk_time_t* t);
int wtk_time_clean(wtk_time_t* t);
int wtk_time_update(wtk_time_t* t);
char* wtk_time_g_now();
#ifdef __cplusplus
};
#endif
#endif
