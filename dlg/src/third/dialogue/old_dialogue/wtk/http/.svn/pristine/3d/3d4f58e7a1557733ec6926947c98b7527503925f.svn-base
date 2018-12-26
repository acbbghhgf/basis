#include "wtk_plink_host_cfg.h"

int wtk_plink_host_cfg_init(wtk_plink_host_cfg_t *cfg)
{
	wtk_httpc_cfg_init(&(cfg->httpc));
	cfg->time_relink=500;
	return 0;
}

int wtk_plink_host_cfg_clean(wtk_plink_host_cfg_t *cfg)
{
	wtk_httpc_cfg_clean(&(cfg->httpc));
	return 0;
}

int wtk_plink_host_cfg_update_local(wtk_plink_host_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_i(lc,cfg,time_relink,v);
	lc=wtk_local_cfg_find_lc_s(main,"httpc");
	if(lc)
	{
		ret=wtk_httpc_cfg_update_local(&(cfg->httpc),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_plink_host_cfg_update(wtk_plink_host_cfg_t *cfg)
{
	int ret;

	ret=wtk_httpc_cfg_update(&(cfg->httpc));
	return ret;
}
