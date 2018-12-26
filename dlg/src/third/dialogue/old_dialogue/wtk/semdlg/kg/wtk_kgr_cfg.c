#include "wtk_kgr_cfg.h" 

int wtk_kgr_cfg_init(wtk_kgr_cfg_t *cfg)
{
	cfg->kg=NULL;
	cfg->kg_fn=NULL;
	cfg->inst_dn=NULL;
	cfg->class_dn=NULL;
	cfg->nlg_fn=NULL;
	cfg->use_random=1;
	cfg->lua_init=NULL;
	cfg->lua_json_map=NULL;
	return 0;
}

int wtk_kgr_cfg_clean(wtk_kgr_cfg_t *cfg)
{
	if(cfg->kg)
	{
		wtk_kg_delete(cfg->kg);
	}
	return 0;
}

int wtk_kgr_cfg_update_local(wtk_kgr_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_b(lc,cfg,use_random,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,nlg_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,kg_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,inst_dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,class_dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_init,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_json_map,v);
	return 0;
}

int wtk_kgr_cfg_update(wtk_kgr_cfg_t *cfg)
{
	cfg->kg=wtk_kg_new_fn(cfg->kg_fn,NULL);
	return cfg->kg?0:-1;
}

int wtk_kgr_cfg_update2(wtk_kgr_cfg_t *cfg,wtk_source_loader_t *sl)
{
	cfg->kg=wtk_kg_new_fn(cfg->kg_fn,(wtk_rbin2_t*)(sl->hook));
	return cfg->kg?0:-1;
}
