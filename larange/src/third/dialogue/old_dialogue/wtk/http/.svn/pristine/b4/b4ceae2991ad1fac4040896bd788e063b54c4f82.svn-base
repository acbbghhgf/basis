#ifndef WTK_NK_WTK_LISTEN_CFG_H_
#define WTK_NK_WTK_LISTEN_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_listen_cfg wtk_listen_cfg_t;
struct wtk_listen_cfg
{
	int backlog;
	int defer_accept_timeout;
	unsigned loop_back:1;
	unsigned reuse:1;
	unsigned defer_accept:1;
};

int wtk_listen_cfg_init(wtk_listen_cfg_t *cfg);
int wtk_listen_cfg_clean(wtk_listen_cfg_t *cfg);
int wtk_listen_cfg_update_local(wtk_listen_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_listen_cfg_update(wtk_listen_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
