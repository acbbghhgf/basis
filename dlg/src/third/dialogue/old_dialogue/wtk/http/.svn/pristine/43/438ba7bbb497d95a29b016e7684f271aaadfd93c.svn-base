#include "wtk_relay_pool_cfg.h"

int wtk_relay_pool_cfg_init(wtk_relay_pool_cfg_t *cfg)
{
	cfg->url.len=0;
	return 0;
}

int wtk_relay_pool_cfg_clean(wtk_relay_pool_cfg_t *cfg)
{
	return 0;
}

int wtk_relay_pool_cfg_update_local(wtk_relay_pool_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,url,v);
	return 0;
}

int wtk_relay_pool_cfg_update(wtk_relay_pool_cfg_t *cfg)
{
	return 0;
}
