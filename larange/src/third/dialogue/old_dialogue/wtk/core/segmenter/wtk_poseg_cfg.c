#include "wtk_poseg_cfg.h" 

int wtk_poseg_cfg_init(wtk_poseg_cfg_t *cfg)
{
	wtk_chnpos_cfg_init(&(cfg->pos));
	cfg->dict=NULL;
	cfg->dict_fn=NULL;
	cfg->filter=NULL;
	cfg->upper=0;
	cfg->max_char=10;
	cfg->def_weight=-40;
	cfg->use_dict_bin=0;
	return 0;
}

int wtk_poseg_cfg_clean(wtk_poseg_cfg_t *cfg)
{
	if(cfg->dict)
	{
		wtk_posdict_delete(cfg->dict);
	}
	wtk_chnpos_cfg_clean(&(cfg->pos));
	return 0;
}

int wtk_poseg_cfg_update_local(wtk_poseg_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_i(lc,cfg,max_char,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,dict_fn,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,upper,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_dict_bin,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,def_weight,v);
	cfg->filter=wtk_local_cfg_find_array_s(lc,"filter");
	lc=wtk_local_cfg_find_lc_s(main,"pos");
	if(lc)
	{
		ret=wtk_chnpos_cfg_update_local(&(cfg->pos),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_poseg_cfg_update(wtk_poseg_cfg_t *cfg)
{
	int ret;

	if(cfg->use_dict_bin)
	{
		cfg->dict=wtk_posdict_new();
		wtk_posdict_set_kv(cfg->dict,cfg->dict_fn);
	}else
	{
		cfg->dict=wtk_posdict_new();
		ret=wtk_source_load_file(cfg->dict,(wtk_source_load_handler_t)wtk_posdict_load,cfg->dict_fn);
		if(ret!=0){goto end;}
	}
	ret=wtk_chnpos_cfg_update(&(cfg->pos));
	if(ret!=0){goto end;}
	ret=0;
end:
	return ret;
}


int wtk_poseg_cfg_update2(wtk_poseg_cfg_t *cfg,wtk_source_loader_t *sl)
{
	int ret;

	if(cfg->use_dict_bin)
	{
		cfg->dict=wtk_posdict_new();
		{
			wtk_rbin2_t *rb=(wtk_rbin2_t*)(sl->hook);
			wtk_posdict_set_kv2(cfg->dict,rb,cfg->dict_fn);
		}
	}else
	{
		cfg->dict=wtk_posdict_new();
		ret=wtk_source_loader_load(sl,cfg->dict,(wtk_source_load_handler_t)wtk_posdict_load,cfg->dict_fn);
		if(ret!=0){goto end;}
	}
	ret=wtk_chnpos_cfg_update2(&(cfg->pos),sl);
	if(ret!=0){goto end;}
	ret=0;
end:
	return ret;
}
