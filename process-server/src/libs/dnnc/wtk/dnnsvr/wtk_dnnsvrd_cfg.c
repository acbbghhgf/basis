#include "wtk_dnnsvrd_cfg.h" 

int wtk_dnnsvrd_cfg_init(wtk_dnnsvrd_cfg_t *cfg)
{
	wtk_daemon_cfg_init(&(cfg->daemon));
	wtk_dnnsvr_cfg_init(&(cfg->dnnsvr));
	return 0;
}

int wtk_dnnsvrd_cfg_clean(wtk_dnnsvrd_cfg_t *cfg)
{
	wtk_daemon_cfg_clean(&(cfg->daemon));
	wtk_dnnsvr_cfg_clean(&(cfg->dnnsvr));
	return 0;
}

int wtk_dnnsvrd_cfg_update_local(wtk_dnnsvrd_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	int ret;

	lc=wtk_local_cfg_find_lc_s(main,"daemon");
	if(lc)
	{
		ret=wtk_daemon_cfg_update_local(&(cfg->daemon),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"dnnsvr");
	if(lc)
	{
		ret=wtk_dnnsvr_cfg_update_local(&(cfg->dnnsvr),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_dnnsvrd_cfg_update(wtk_dnnsvrd_cfg_t *cfg)
{
	int ret;

	ret=wtk_daemon_cfg_update(&(cfg->daemon));
	if(ret!=0){goto end;}
	ret=wtk_dnnsvr_cfg_update(&(cfg->dnnsvr));
	if(ret!=0){goto end;}
end:
	return ret;
}

void wtk_dnnsvrd_cfg_update_arg(wtk_dnnsvrd_cfg_t *cfg,wtk_arg_t *arg)
{
	wtk_daemon_cfg_update_arg(&(cfg->daemon),arg);
	wtk_dnnsvr_cfg_update_arg(&(cfg->dnnsvr),arg);
}
