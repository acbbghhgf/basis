#ifndef WTK_OS_WTK_LOG_CFG_H_
#define WTK_OS_WTK_LOG_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/core/wtk_arg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_log_cfg wtk_log_cfg_t;
typedef enum
{
	wtk_log_notice=0,
#define LOG_NOTICE 0
	wtk_log_warn=1,
#define LOG_WARN 1
	wtk_log_err=2,
#define LOG_ERR 2
}wtk_log_level_t;

struct wtk_log_cfg
{
	char log_fn[256];
	wtk_string_t fn;
	wtk_log_level_t level;
	unsigned fn_pad_timestamp:1;
	unsigned use_console:1;
	unsigned log_ts:1;
	unsigned log_touch:1;
    unsigned daily:1;
};

int wtk_log_cfg_init(wtk_log_cfg_t *cfg);
int wtk_log_cfg_clean(wtk_log_cfg_t *cfg);
int wtk_log_cfg_update_local(wtk_log_cfg_t *cfg,wtk_local_cfg_t *lc);
void wtk_log_cfg_update_arg(wtk_log_cfg_t *cfg,wtk_arg_t *arg);
int wtk_log_cfg_update(wtk_log_cfg_t *cfg);
struct wtk_log* wtk_log_cfg_new_log(wtk_log_cfg_t *cfg);
void wtk_log_cfg_print_usage();
#ifdef __cplusplus
};
#endif
#endif
