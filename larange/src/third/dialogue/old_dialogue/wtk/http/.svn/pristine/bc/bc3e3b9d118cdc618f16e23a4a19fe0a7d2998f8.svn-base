#ifndef WTK_HTTP_LOC_REDIRECT_POOL_WTK_RELAY_POOL_CFG_H_
#define WTK_HTTP_LOC_REDIRECT_POOL_WTK_RELAY_POOL_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_relay_pool_cfg wtk_relay_pool_cfg_t;
struct wtk_relay_pool_cfg
{
	wtk_string_t url;
};

int wtk_relay_pool_cfg_init(wtk_relay_pool_cfg_t *cfg);
int wtk_relay_pool_cfg_clean(wtk_relay_pool_cfg_t *cfg);
int wtk_relay_pool_cfg_update_local(wtk_relay_pool_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_relay_pool_cfg_update(wtk_relay_pool_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
