#include "wtk_chnlike_cfg.h" 

int wtk_chnlike_cfg_init(wtk_chnlike_cfg_t *cfg)
{
	wtk_txtpeek_cfg_init(&(cfg->txtpeek));
	wtk_strlike_cfg_init(&(cfg->strlike));
	cfg->dict_fn=NULL;
	cfg->hash_hint=15003;
	cfg->dict=NULL;
	cfg->use_bin=0;
	return 0;
}

int wtk_chnlike_cfg_clean(wtk_chnlike_cfg_t *cfg)
{
	if(cfg->dict)
	{
		wtk_strdict_delete(cfg->dict);
	}
	wtk_txtpeek_cfg_clean(&(cfg->txtpeek));
	wtk_strlike_cfg_clean(&(cfg->strlike));
	return 0;
}

int wtk_chnlike_cfg_update_local(wtk_chnlike_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_i(lc,cfg,hash_hint,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_bin,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,dict_fn,v);
	lc=wtk_local_cfg_find_lc_s(main,"txtpeek");
	if(lc)
	{
		ret=wtk_txtpeek_cfg_update_local(&(cfg->txtpeek),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"strlike");
	if(lc)
	{
		ret=wtk_strlike_cfg_update_local(&(cfg->strlike),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_chnlike_cfg_update(wtk_chnlike_cfg_t *cfg)
{
	int ret;

	if(cfg->dict_fn && !cfg->use_bin)
	{
		cfg->dict=wtk_strdict_new(cfg->hash_hint);
		ret=wtk_source_load_file(cfg->dict,(wtk_source_load_handler_t)wtk_strdict_load,cfg->dict_fn);
		if(ret!=0){goto end;}
	}
	ret=wtk_txtpeek_cfg_update(&(cfg->txtpeek));
	if(ret!=0){goto end;}
	ret=wtk_strlike_cfg_update(&(cfg->strlike));
	if(ret!=0){goto end;}
end:
	return ret;
}
