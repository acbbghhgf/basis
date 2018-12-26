#include "wtk_faqc_cfg.h" 

int wtk_faqc_cfg_init(wtk_faqc_cfg_t *cfg)
{
	wtk_httpc_cfg_init(&(cfg->httpc));
	wtk_string_set(&(cfg->appid),0,0);
	cfg->conf = 0.8;
	return 0;
}

int wtk_faqc_cfg_clean(wtk_faqc_cfg_t *cfg)
{
	wtk_httpc_cfg_clean(&(cfg->httpc));
	return 0;
}

int wtk_faqc_cfg_update_local(wtk_faqc_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_string_v(lc,cfg,appid,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,conf,v);
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

int wtk_faqc_cfg_update(wtk_faqc_cfg_t *cfg)
{
	int ret;

	ret=wtk_httpc_cfg_update(&(cfg->httpc));
	if(ret!=0){goto end;}
	ret=0;
end:
	return ret;
}
