#ifndef WTK_NK_WTK_NK_CFG
#define WTK_NK_WTK_NK_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/os/wtk_log_cfg.h"
#include "wtk/os/wtk_log.h"
#include "wtk/nk/poll/wtk_poll.h"
#include "wtk/core/cfg/wtk_opt.h"
#include "wtk_listen_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nk_cfg wtk_nk_cfg_t;
struct wtk_nk_cfg
{
	wtk_log_cfg_t log;
	wtk_poll_cfg_t poll;
	wtk_listen_cfg_t listen;
	int rw_size;
	int cache_size;
	int con_buf_size;
	int max_connection;
	unsigned debug:1;
	unsigned log_snd:1;
	unsigned log_rcv:1;
};

int wtk_nk_cfg_init(wtk_nk_cfg_t *cfg);
int wtk_nk_cfg_clean(wtk_nk_cfg_t *cfg);
int wtk_nk_cfg_update_local(wtk_nk_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_nk_cfg_update(wtk_nk_cfg_t *cfg);
void wtk_nk_cfg_update_arg(wtk_nk_cfg_t *cfg,wtk_arg_t *arg);
#ifdef __cplusplus
};
#endif
#endif
