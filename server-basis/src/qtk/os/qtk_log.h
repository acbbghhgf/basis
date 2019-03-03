#ifndef QTK_OS_QTK_LOG_H_
#define QTK_OS_QTK_LOG_H_
#include "wtk/core/wtk_type.h"
#include "wtk/os/wtk_time.h"
#include "wtk/os/wtk_log_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_log qtk_log_t;
#define qtk_log_log(l,fmt,...) (l ? qtk_log_printf(l,__FUNCTION__,__LINE__,LOG_NOTICE,fmt,__VA_ARGS__) : 0)
#define qtk_log_log0(l,fmt) (l? qtk_log_printf(l,__FUNCTION__,__LINE__,LOG_NOTICE,fmt):0)
#define qtk_log_warn(l,fmt,...) qtk_log_printf(l,__FUNCTION__,__LINE__,LOG_WARN,fmt,__VA_ARGS__)
#define qtk_log_warn0(l,fmt) qtk_log_printf(l,__FUNCTION__,__LINE__,LOG_WARN,fmt)
#define qtk_log_err(l,fmt,...) qtk_log_printf(l,__FUNCTION__,__LINE__,LOG_ERR,fmt,__VA_ARGS__)
#define qtk_log_err0(l,fmt) qtk_log_printf(l,__FUNCTION__,__LINE__,LOG_ERR,fmt)

struct qtk_log
{
    char fn[256];
    FILE* f;
    wtk_time_t* t;
    int today; /* month day */
    pthread_spinlock_t l;
    int log_level;
    unsigned log_ts:1;
    unsigned daily:1;
    unsigned log_touch:1;
};

extern qtk_log_t *glb_qlog;

int qtk_log_reopen(qtk_log_t *l); /* for subprocess called _inst_reopen_fds */
qtk_log_t* qtk_log_new(char *fn);
qtk_log_t* qtk_log_new2(char *fn,int log_level);
qtk_log_t* qtk_log_new3(char *fn,int log_level, int daily);
int qtk_log_delete(qtk_log_t *log);
int qtk_log_init(qtk_log_t* l,char* fn);
int qtk_log_redirect(qtk_log_t *l,char *fn);
int qtk_log_clean(qtk_log_t* l);
int qtk_log_printf(qtk_log_t* l,const char *func,int line,int level,char* fmt,...);
double qtk_log_cur_time(qtk_log_t *l);

#ifdef __cplusplus
};
#endif
#endif
