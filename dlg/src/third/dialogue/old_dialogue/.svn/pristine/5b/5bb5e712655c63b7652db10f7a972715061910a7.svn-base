#include "wtk_prune_cfg.h" 

int wtk_prune_cfg_init(wtk_prune_cfg_t *cfg)
{
	cfg->min_score=0;
	cfg->max_score=0;
	cfg->bin_width=1.0;
	cfg->count=0;
	cfg->beam=0;
	return 0;
}

int wtk_prune_cfg_clean(wtk_prune_cfg_t *cfg)
{
	return 0;
}

int wtk_prune_cfg_update_local(wtk_prune_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_f(lc,cfg,min_score,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,max_score,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,bin_width,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,beam,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,count,v);
	return 0;
}

int wtk_prune_cfg_update(wtk_prune_cfg_t *cfg)
{
	return 0;
}
