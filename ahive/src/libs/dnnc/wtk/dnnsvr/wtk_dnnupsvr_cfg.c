#include "wtk_dnnupsvr_cfg.h" 

int wtk_dnnupsvr_cfg_init(wtk_dnnupsvr_cfg_t *cfg)
{
	wtk_cudnn_cfg_init(&(cfg->dnn));
	cfg->thread_per_gpu=3;
	cfg->cache_size=100;
	cfg->debug=0;
	return 0;
}

int wtk_dnnupsvr_cfg_clean(wtk_dnnupsvr_cfg_t *cfg)
{
	wtk_cudnn_cfg_clean(&(cfg->dnn));
	return 0;
}

int wtk_dnnupsvr_cfg_update_local(wtk_dnnupsvr_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_string_t *v;
	wtk_local_cfg_t *lc;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_b(lc,cfg,debug,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,cache_size,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,thread_per_gpu,v);
	lc=wtk_local_cfg_find_lc_s(main,"dnn");
	if(lc)
	{
		ret=wtk_cudnn_cfg_update_local(&(cfg->dnn),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_dnnupsvr_cfg_update(wtk_dnnupsvr_cfg_t *cfg)
{
	int ret;

	ret=wtk_cudnn_cfg_update(&(cfg->dnn));
	if(ret!=0){goto end;}
	ret=0;
end:
	return ret;
}

void wtk_dnnupsvr_cfg_print(wtk_dnnupsvr_cfg_t *cfg)
{
	wtk_cudnn_cfg_print(&(cfg->dnn));
	printf("thread_per_gpu: %d\n",cfg->thread_per_gpu);
}
