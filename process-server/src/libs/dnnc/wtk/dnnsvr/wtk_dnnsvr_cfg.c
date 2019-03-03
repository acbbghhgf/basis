#include "wtk_dnnsvr_cfg.h" 
wtk_cudnn_gpu_cfg_t* wtk_cudnn_gpu_cfg_new(int idx);

int wtk_dnnsvr_cfg_init(wtk_dnnsvr_cfg_t *cfg)
{
	wtk_nk_cfg_init(&(cfg->nk));
	wtk_dnnupsvr_cfg_init(&(cfg->upsvr));
	return 0;
}

int wtk_dnnsvr_cfg_clean(wtk_dnnsvr_cfg_t *cfg)
{
	wtk_nk_cfg_clean(&(cfg->nk));
	wtk_dnnupsvr_cfg_clean(&(cfg->upsvr));
	return 0;
}

int wtk_dnnsvr_cfg_update_local(wtk_dnnsvr_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	int ret;

	lc=wtk_local_cfg_find_lc_s(main,"nk");
	if(lc)
	{
		ret=wtk_nk_cfg_update_local(&(cfg->nk),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"upsvr");
	if(lc)
	{
		ret=wtk_dnnupsvr_cfg_update_local(&(cfg->upsvr),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_dnnsvr_cfg_update(wtk_dnnsvr_cfg_t *cfg)
{
	int ret;

	ret=wtk_nk_cfg_update(&(cfg->nk));
	if(ret!=0){goto end;}
	ret=wtk_dnnupsvr_cfg_update(&(cfg->upsvr));
	if(ret!=0){goto end;}
end:
	return ret;
}

void wtk_dnnsvr_cfg_update_arg(wtk_dnnsvr_cfg_t *cfg,wtk_arg_t *arg)
{
	wtk_nk_cfg_update_arg(&(cfg->nk),arg);
	wtk_dnnsvr_cfg_print(cfg);
}

void wtk_dnnsvr_cfg_print(wtk_dnnsvr_cfg_t *cfg)
{
	wtk_dnnupsvr_cfg_print(&(cfg->upsvr));
}
