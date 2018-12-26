#ifndef WTK_OS_TCP_WTK_TCP_LISTEN_CFG
#define WTK_OS_TCP_WTK_TCP_LISTEN_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_tcp_listen_cfg wtk_tcp_listen_cfg_t;
struct wtk_tcp_listen_cfg
{
	int port;
	int backlog;
	unsigned reuse:1;
};

int wtk_tcp_listen_cfg_init(wtk_tcp_listen_cfg_t *cfg);
int wtk_tcp_listen_cfg_clean(wtk_tcp_listen_cfg_t *cfg);
int wtk_tcp_listen_cfg_update_local(wtk_tcp_listen_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_tcp_listen_cfg_update(wtk_tcp_listen_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
