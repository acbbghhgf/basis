#include "wtk_semfld_cfg.h" 

int wtk_semfld_cfg_init(wtk_semfld_cfg_t *cfg)
{
	wtk_string_set(&(cfg->name),0,0);
	wtk_string_set(&(cfg->ename),0,0);
	wtk_semfi_cfg_init(&(cfg->semfi));
	wtk_semslot_cfg_init(&(cfg->slot));
	wtk_kgr_cfg_init(&(cfg->kg));
	cfg->luafn=NULL;
	cfg->nlgfn=NULL;
	cfg->fstfn=NULL;
	cfg->owlfn=NULL;
	cfg->nlg=NULL;
	cfg->nlg_main=NULL;
	cfg->nlg_usr_inform=NULL;
	cfg->nlg_usr_set=NULL;
	cfg->nlg_usr_del=NULL;
	cfg->nlg_sys_ask=NULL;
	cfg->nlg_sys_answer=NULL;
	cfg->max_empty_input_try=1;
	cfg->dn=NULL;
	cfg->owl=NULL;
	cfg->lua_flush=NULL;
	cfg->lua_env_init=NULL;
	cfg->use_kg=0;
	cfg->dat_dn=NULL;
	cfg->lex_pre_dat=NULL;
	cfg->use_dat_env=1;
	cfg->use_nlg2=0;
	wtk_string_set(&(cfg->nlg_close),0,0);
	return 0;
}

int wtk_semfld_cfg_clean(wtk_semfld_cfg_t *cfg)
{
	if(cfg->lex_pre_dat)
	{
		wtk_string_delete(cfg->lex_pre_dat);
	}
	if(cfg->use_kg)
	{
		wtk_kgr_cfg_clean(&(cfg->kg));
	}
	if(cfg->owl)
	{
		wtk_owl_tree_delete(cfg->owl);
	}
	if(cfg->nlg)
	{
		wtk_nlg_delete(cfg->nlg);
	}
	wtk_semfi_cfg_clean(&(cfg->semfi));
	wtk_semslot_cfg_clean(&(cfg->slot));
	return 0;
}

int wtk_semfld_cfg_update_local(wtk_semfld_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	//wtk_local_cfg_print(main);
	lc=main;
	wtk_local_cfg_update_cfg_string_v(lc,cfg,name,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,ename,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,nlg_close,v);
	if(cfg->ename.len==0)
	{
		cfg->ename=lc->name;
	}
	if(cfg->name.len==0)
	{
		cfg->name=lc->name;
	}
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_flush,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_env_init,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,dat_dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,luafn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,nlgfn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,fstfn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,owlfn,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,max_empty_input_try,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_kg,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_nlg2,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_dat_env,v);
	lc=wtk_local_cfg_find_lc_s(main,"semfi");
	if(lc)
	{
		ret=wtk_semfi_cfg_update_local(&(cfg->semfi),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"slot");
	if(lc)
	{
		ret=wtk_semslot_cfg_update_local(&(cfg->slot),lc);
		if(ret!=0){goto end;}
	}
	if(cfg->use_kg)
	{
		lc=wtk_local_cfg_find_lc_s(main,"kg");
		if(lc)
		{
			ret=wtk_kgr_cfg_update_local(&(cfg->kg),lc);
			if(ret!=0){goto end;}
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_semfld_cfg_update(wtk_semfld_cfg_t *cfg,wtk_lexc_t *lex)
{
	int ret;

	if(cfg->lex_pre_dat)
	{
		lex->pre_dat=cfg->lex_pre_dat;
	}
	ret=wtk_semfi_cfg_update(&(cfg->semfi),lex);
	if(cfg->lex_pre_dat)
	{
		lex->pre_dat=NULL;
	}
	if(ret!=0){goto end;}
	if(!cfg->fstfn && cfg->nlgfn)
	{
		cfg->nlg=wtk_nlg_new(cfg->nlgfn);
		if(!cfg->nlg){ret=-1;goto end;}
		//wtk_debug("%s\n",cfg->nlgfn);
		//wtk_nlg_print(cfg->nlg);
		cfg->nlg_main=wtk_nlg_get_root_s(cfg->nlg,"main");
		cfg->nlg_usr_inform=wtk_nlg_get_root_s(cfg->nlg,"usr_inform");
		cfg->nlg_usr_set=wtk_nlg_get_root_s(cfg->nlg,"usr_set");
		cfg->nlg_usr_del=wtk_nlg_get_root_s(cfg->nlg,"usr_del");
		cfg->nlg_sys_ask=wtk_nlg_get_root_s(cfg->nlg,"sys_ask");
		cfg->nlg_sys_answer=wtk_nlg_get_root_s(cfg->nlg,"sys_answer");
	}
	if(cfg->use_kg)
	{
		ret=wtk_kgr_cfg_update(&(cfg->kg));
		if(ret!=0){goto end;}
	}
	if(cfg->owlfn)
	{
		cfg->owl=wtk_owl_tree_new_fn(cfg->owlfn);
	}
	ret=wtk_semslot_cfg_update(&(cfg->slot));
	if(ret!=0){goto end;}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}


int wtk_semfld_cfg_update2(wtk_semfld_cfg_t *cfg,wtk_source_loader_t *sl,wtk_lexc_t *lex)
{
	int ret;

	//wtk_debug("update %p/%p...\n",cfg->nlgfn,cfg->luafn);
	if(cfg->lex_pre_dat)
	{
		lex->pre_dat=cfg->lex_pre_dat;
	}
	ret=wtk_semfi_cfg_update2(&(cfg->semfi),sl,lex);
	if(cfg->lex_pre_dat)
	{
		lex->pre_dat=NULL;
	}
	if(ret!=0){goto end;}
	if(!cfg->fstfn && cfg->nlgfn)
	{
		cfg->nlg=wtk_nlg_new2((wtk_rbin2_t*)(sl->hook),cfg->nlgfn);
		if(!cfg->nlg){ret=-1;goto end;}
		//wtk_debug("%s\n",cfg->nlgfn);
		//wtk_nlg_print(cfg->nlg);
		cfg->nlg_main=wtk_nlg_get_root_s(cfg->nlg,"main");
		cfg->nlg_usr_inform=wtk_nlg_get_root_s(cfg->nlg,"usr_inform");
		cfg->nlg_usr_set=wtk_nlg_get_root_s(cfg->nlg,"usr_set");
		cfg->nlg_usr_del=wtk_nlg_get_root_s(cfg->nlg,"usr_del");
		cfg->nlg_sys_ask=wtk_nlg_get_root_s(cfg->nlg,"sys_ask");
		cfg->nlg_sys_answer=wtk_nlg_get_root_s(cfg->nlg,"sys_answer");
	}
	if(cfg->use_kg)
	{
		ret=wtk_kgr_cfg_update2(&(cfg->kg),sl);
		if(ret!=0){goto end;}
	}
	if(cfg->owlfn)
	{
		cfg->owl=wtk_owl_tree_new_fn2((wtk_rbin2_t*)(sl->hook),cfg->owlfn);
	}
	ret=wtk_semslot_cfg_update(&(cfg->slot));
	if(ret!=0){goto end;}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

