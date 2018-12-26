#ifndef WTK_OS_DAEMON_WTK_DAEMON_CFG_H_
#define WTK_OS_DAEMON_WTK_DAEMON_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/os/wtk_proc.h"
#include "wtk/os/wtk_pid.h"
#include "wtk/core/wtk_arg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_daemon_cfg wtk_daemon_cfg_t;

struct wtk_daemon_cfg
{
	wtk_strbuf_t *fn;
	wtk_string_t pid_fn;			//http.weasel.pid, http is pid_fn,and fn is http.weasel.pid;
	unsigned daemon:1;
	unsigned debug:1;
	unsigned detect_running:1;
};

int wtk_daemon_cfg_init(wtk_daemon_cfg_t *cfg);
int wtk_daemon_cfg_clean(wtk_daemon_cfg_t *cfg);
int wtk_daemon_cfg_update_local(wtk_daemon_cfg_t *cfg,wtk_local_cfg_t *lc);
void wtk_daemon_cfg_update_arg(wtk_daemon_cfg_t *cfg,wtk_arg_t *arg);
int wtk_daemon_cfg_update(wtk_daemon_cfg_t *cfg);
void wtk_daemon_cfg_init_with_main(wtk_daemon_cfg_t *cfg,wtk_local_cfg_t *main);

/**
 * @brief delete daemon process is running or not(use pid file).
 */
int wtk_daemon_cfg_is_running(wtk_daemon_cfg_t *cfg);


/**
 * @brief check daemon process is running or not. if allow multi-daemon process,generate new pid file.
 * @return 0 on success else failed.
 */
int wtk_daemon_cfg_recheck(wtk_daemon_cfg_t *cfg);
void wtk_daemon_cfg_print_usage();
#ifdef __cplusplus
};
#endif
#endif
