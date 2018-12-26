#include "wtk_zip_cfg.h" 

int wtk_zip_cfg_init(wtk_zip_cfg_t *cfg)
{
	cfg->blk_size=1024;
	cfg->bits=13;
	return 0;
}

int wtk_zip_cfg_clean(wtk_zip_cfg_t *cfg)
{
	return 0;
}

int wtk_zip_cfg_update_local(wtk_zip_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_i(lc,cfg,blk_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,bits,v);
	return 0;
}

int wtk_zip_cfg_update(wtk_zip_cfg_t *cfg)
{
	return 0;
}
