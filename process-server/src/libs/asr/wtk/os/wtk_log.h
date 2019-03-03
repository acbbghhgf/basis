#ifndef WTK_MAIN_WTK_LOG_H_
#define WTK_MAIN_WTK_LOG_H_
#include "wtk/core/wtk_type.h"
#include "wtk/os/wtk_time.h"
#include "wtk/os/wtk_lock.h"
#include "wtk_log_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_log wtk_log_t;
#define wtk_log_log(l,fmt,...) (l ? wtk_log_printf(l,__FUNCTION__,__LINE__,LOG_NOTICE,fmt,__VA_ARGS__) : 0)
#define wtk_log_log0(l,fmt) (l? wtk_log_printf(l,__FUNCTION__,__LINE__,LOG_NOTICE,fmt):0)
#define wtk_log_warn(l,fmt,...) wtk_log_printf(l,__FUNCTION__,__LINE__,LOG_WARN,fmt,__VA_ARGS__)
#define wtk_log_warn0(l,fmt) wtk_log_printf(l,__FUNCTION__,__LINE__,LOG_WARN,fmt)
#define wtk_log_err(l,fmt,...) wtk_log_printf(l,__FUNCTION__,__LINE__,LOG_ERR,fmt,__VA_ARGS__)
#define wtk_log_err0(l,fmt) wtk_log_printf(l,__FUNCTION__,__LINE__,LOG_ERR,fmt)

struct wtk_log
{
    char fn[256];
	FILE* f;
	wtk_time_t* t;
    int today; /* month day */
	wtk_lock_t l;
	int log_level;
	unsigned log_ts:1;
    unsigned daily:1;
    unsigned log_touch:1;
};

extern wtk_log_t *glb_log;

wtk_log_t* wtk_log_new(char *fn);
wtk_log_t* wtk_log_new2(char *fn,int log_level);
wtk_log_t* wtk_log_new3(char *fn,int log_level, int daily);
int wtk_log_delete(wtk_log_t *log);
int wtk_log_init(wtk_log_t* l,char* fn);
int wtk_log_redirect(wtk_log_t *l,char *fn);
int wtk_log_clean(wtk_log_t* l);
int wtk_log_printf(wtk_log_t* l,const char *func,int line,int level,char* fmt,...);
int wtk_log_g_print_time(char *buf);
double wtk_log_cur_time(wtk_log_t *l);

//--------------------- test/example section ----------------
void wtk_log_test_g();
#define wtk_log_print(s) wtk_log_log0(glb_log,s)
#ifdef __cplusplus
};
#endif
#endif
