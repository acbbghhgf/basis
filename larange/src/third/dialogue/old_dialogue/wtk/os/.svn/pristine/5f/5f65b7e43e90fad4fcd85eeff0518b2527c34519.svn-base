#include "wtk_dtp_cfg.h"

int wtk_dtp_cfg_init(wtk_dtp_cfg_t *cfg)
{
	cfg->min=wtk_get_cpus();
	cfg->max=-1;
	cfg->timeout=300;
	return 0;
}

int wtk_dtp_cfg_clean(wtk_dtp_cfg_t *cfg)
{
	return 0;
}

int wtk_dtp_cfg_update_local(wtk_dtp_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_i(lc,cfg,min,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,timeout,v);
	return 0;
}

int wtk_dtp_cfg_update(wtk_dtp_cfg_t *cfg)
{
	return 0;
}
