#include "wtk_txtpeek_cfg.h" 

int wtk_txtpeek_cfg_init(wtk_txtpeek_cfg_t *cfg)
{
	cfg->merge_num=1;
	cfg->skip_oov=1;
	return 0;
}

int wtk_txtpeek_cfg_clean(wtk_txtpeek_cfg_t *cfg)
{
	return 0;
}

int wtk_txtpeek_cfg_update_local(wtk_txtpeek_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_b(lc,cfg,skip_oov,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,merge_num,v);
	return 0;
}

int wtk_txtpeek_cfg_update(wtk_txtpeek_cfg_t *cfg)
{
	return 0;
}
