#include "wtk_clsvec_cfg.h" 

int wtk_clsvec_cfg_init(wtk_clsvec_cfg_t *cfg)
{
	cfg->use_idx=0;
	return 0;
}

int wtk_clsvec_cfg_clean(wtk_clsvec_cfg_t *cfg)
{
	return 0;
}

int wtk_clsvec_cfg_update_local(wtk_clsvec_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_b(lc,cfg,use_idx,v);
	return 0;
}

int wtk_clsvec_cfg_update(wtk_clsvec_cfg_t *cfg)
{
	return 0;
}
