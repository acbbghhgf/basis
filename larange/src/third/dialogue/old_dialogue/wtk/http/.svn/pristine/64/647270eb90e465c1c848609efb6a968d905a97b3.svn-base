#include "wtk_nbc_cfg.h"

int wtk_nbc_cfg_init(wtk_nbc_cfg_t *cfg)
{
	wtk_nk_cfg_init(&(cfg->nk));
	//wtk_httpnc_cfg_init(&(cfg->httpnc));
	return 0;
}

int wtk_nbc_cfg_clean(wtk_nbc_cfg_t *cfg)
{
	wtk_nk_cfg_clean(&(cfg->nk));
	//wtk_httpnc_cfg_clean(&(cfg->httpnc));
	return 0;
}

int wtk_nbc_cfg_update_local(wtk_nbc_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	int ret;

	lc=wtk_local_cfg_find_lc_s(main,"nk");
	if(!lc)
	{
		ret=wtk_nk_cfg_update_local(&(cfg->nk),lc);
		if(ret!=0){goto end;}
	}
	/*
	lc=wtk_local_cfg_find_lc_s(main,"httpnc");
	if(lc)
	{
		ret=wtk_httpnc_cfg_update_local(&(cfg->httpnc),lc);
		if(ret!=0){goto end;}
	}*/
	ret=0;
end:
	return ret;
}

int wtk_nbc_cfg_update(wtk_nbc_cfg_t *cfg)
{
	int ret;

	ret=wtk_nk_cfg_update(&(cfg->nk));
	if(ret!=0){goto end;}
	/*
	ret=wtk_httpnc_cfg_update(&(cfg->httpnc));
	if(ret!=0){goto end;}
	*/
end:
	return ret;
}
