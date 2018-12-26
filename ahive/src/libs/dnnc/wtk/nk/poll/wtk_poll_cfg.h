#ifndef WTK_NK_POLL_WTK_POLL_CFG
#define WTK_NK_POLL_WTK_POLL_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_epoll_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_poll_cfg wtk_poll_cfg_t;
struct wtk_poll_cfg
{
	wtk_epoll_cfg_t epoll;
	int timeout;
	unsigned use_epoll:1;
};

int wtk_poll_cfg_init(wtk_poll_cfg_t *cfg);
int wtk_poll_cfg_clean(wtk_poll_cfg_t *cfg);
int wtk_poll_cfg_update_local(wtk_poll_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_poll_cfg_update(wtk_poll_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
