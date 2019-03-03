#ifndef WTK_NK_POLL_WTK_EPOLL_CFG
#define WTK_NK_POLL_WTK_EPOLL_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_epoll_cfg wtk_epoll_cfg_t;
struct wtk_epoll_cfg
{
	int size;
	unsigned use_et:1;
};

int wtk_epoll_cfg_init(wtk_epoll_cfg_t *cfg);
int wtk_epoll_cfg_clean(wtk_epoll_cfg_t *cfg);
int wtk_epoll_cfg_update_local(wtk_epoll_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_epoll_cfg_update(wtk_epoll_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
