#ifndef WTK_HTTP_LOC_REDIRECT_WTK_REDIRECT_CFG_H_
#define WTK_HTTP_LOC_REDIRECT_WTK_REDIRECT_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/loc/redirect/host/wtk_relay_host.h"
#include "wtk/http/loc/redirect/pool/wtk_relay_pool.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_redirect_cfg wtk_redirect_cfg_t;

struct wtk_redirect_cfg
{
	wtk_relay_host_cfg_t *hosts;
	int nhost;
	wtk_relay_pool_cfg_t *pools;
	int npool;
	wtk_relay_pool_t **xpools;
};

int wtk_redirect_cfg_init(wtk_redirect_cfg_t *cfg);
int wtk_redirect_cfg_clean(wtk_redirect_cfg_t *cfg);
int wtk_redirect_cfg_update_local(wtk_redirect_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_redirect_cfg_update(wtk_redirect_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
