#include "wtk_chnpos_cfg.h"
#include "wtk/core/cfg/wtk_source.h"

int wtk_chnpos_cfg_init(wtk_chnpos_cfg_t *cfg)
{
	wtk_prune_cfg_init(&(cfg->prune));
	cfg->model=NULL;
	cfg->model_fn=NULL;
	cfg->use_prune=0;
	return 0;
}

int wtk_chnpos_cfg_clean(wtk_chnpos_cfg_t *cfg)
{
	if(cfg->model)
	{
		wtk_chnpos_model_delete(cfg->model);
	}
	return 0;
}

int wtk_chnpos_cfg_update_local(wtk_chnpos_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_string_t *v;
	wtk_local_cfg_t *lc;

	lc=main;
	wtk_local_cfg_update_cfg_str(lc,cfg,model_fn,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_prune,v);
	lc=wtk_local_cfg_find_lc_s(main,"prune");
	if(lc)
	{
		wtk_prune_cfg_update_local(&(cfg->prune),lc);
	}
	return 0;
}

int wtk_chnpos_cfg_update(wtk_chnpos_cfg_t *cfg)
{
	wtk_prune_cfg_update(&(cfg->prune));
	cfg->model=wtk_chnpos_model_new();
	wtk_source_load_file(cfg->model,(wtk_source_load_handler_t)wtk_chnpos_model_load,cfg->model_fn);
	return 0;
}

int wtk_chnpos_cfg_update2(wtk_chnpos_cfg_t *cfg,wtk_source_loader_t *sl)
{
	int ret;

	wtk_prune_cfg_update(&(cfg->prune));
	cfg->model=wtk_chnpos_model_new();
	ret=wtk_source_loader_load(sl,cfg->model,(wtk_source_load_handler_t)wtk_chnpos_model_load,cfg->model_fn);
	return ret;
}
