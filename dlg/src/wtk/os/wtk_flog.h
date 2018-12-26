#ifndef WTK_OS_WTK_FLOG_H_
#define WTK_OS_WTK_FLOG_H_
#include "wtk/core/wtk_type.h"
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_flog wtk_flog_t;

typedef enum
{
	WTK_FLOG_OPEN,
	WTK_FLOG_FLUSH,
	WTK_FLOG_CLOSE,
}wtk_flog_state_t;
typedef int (*wtk_flog_write_cb_f)(void *app_data,wtk_flog_state_t state,wtk_flog_t *f);

struct wtk_flog
{
	FILE* file;
	int writed;
	int max_pend;
	int pending;
	wtk_flog_write_cb_f write_cb;
	void *app_data;
};

wtk_flog_t* wtk_flog_new(int max_pend);
int wtk_flog_delete(wtk_flog_t *f);
int wtk_flog_open(wtk_flog_t *f,char *fn,void *cb_hook,wtk_flog_write_cb_f cb);
int wtk_flog_close(wtk_flog_t *f);
int wtk_flog_write(wtk_flog_t *f,const char *data,int bytes);
int wtk_flog_flush(wtk_flog_t *f);
#ifdef __cplusplus
};
#endif
#endif
