#ifdef USE_CRF
#include "wtk_actkg_cfg.h" 

int wtk_actkg_cfg_init(wtk_actkg_cfg_t *cfg)
{
	wtk_crfact_parser_cfg_init(&(cfg->crfact));
	wtk_owlkv_cfg_init(&(cfg->owlkv));
	cfg->vt_lex_fn=NULL;
	cfg->fst_fn=NULL;
	cfg->nlg_fn=NULL;
	cfg->lua_fn=NULL;
	cfg->net_vt=NULL;
	cfg->v_expand_fn=NULL;
	cfg->rt_lex_fn=NULL;
	cfg->rrt_lex_fn=NULL;
	cfg->net_rt=NULL;
	cfg->net_rrt=NULL;
	cfg->net_parser=NULL;
	cfg->parser_lex_fn=NULL;
	cfg->pa_lex_fn=NULL;
	cfg->nhist=5;
	cfg->lua_usr_get=NULL;
	return 0;
}

int wtk_actkg_cfg_clean(wtk_actkg_cfg_t *cfg)
{
	if(cfg->net_vt)
	{
		wtk_lex_net_delete(cfg->net_vt);
	}
	if(cfg->net_rt)
	{
		wtk_lex_net_delete(cfg->net_rt);
	}
	if(cfg->net_rrt)
	{
		wtk_lex_net_delete(cfg->net_rrt);
	}
	if(cfg->net_parser)
	{
		wtk_lex_net_delete(cfg->net_parser);
	}
	if(cfg->net_pa)
	{
		wtk_lex_net_delete(cfg->net_pa);
	}
	wtk_owlkv_cfg_clean(&(cfg->owlkv));
	wtk_crfact_parser_cfg_clean(&(cfg->crfact));
	return 0;
}

int wtk_actkg_cfg_update_local(wtk_actkg_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_usr_get,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,pa_lex_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,vt_lex_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,rt_lex_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,rrt_lex_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,parser_lex_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,fst_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,nlg_fn,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,nhist,v);
	//wtk_local_cfg_update_cfg_str(lc,cfg,lua_fn,v);
	cfg->lua_fn=wtk_local_cfg_find_array_s(lc,"lua_fn");
	wtk_local_cfg_update_cfg_str(lc,cfg,v_expand_fn,v);
	lc=wtk_local_cfg_find_lc_s(main,"crfact");
	if(lc)
	{
		ret=wtk_crfact_parser_cfg_update_local(&(cfg->crfact),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"owlkv");
	if(lc)
	{
		ret=wtk_owlkv_cfg_update_local(&(cfg->owlkv),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_actkg_cfg_update(wtk_actkg_cfg_t *cfg)
{
	int ret;

	ret=wtk_crfact_parser_cfg_update(&(cfg->crfact));
	if(ret!=0){goto end;}
	ret=wtk_owlkv_cfg_update(&(cfg->owlkv));
	if(ret!=0){goto end;}
	ret=0;
end:
	return ret;
}

int wtk_actkg_cfg_update_lex(wtk_actkg_cfg_t *cfg,wtk_lexc_t *lex)
{
	int ret=-1;

	if(cfg->vt_lex_fn)
	{
		cfg->net_vt=wtk_lexc_compile_file2(lex,cfg->vt_lex_fn);
		if(!cfg->net_vt)
		{
			wtk_debug("compile %s failed.\n",cfg->vt_lex_fn);
			goto end;
		}
	}
	if(cfg->rt_lex_fn)
	{
		cfg->net_rt=wtk_lexc_compile_file2(lex,cfg->rt_lex_fn);
		if(!cfg->net_rt)
		{
			wtk_debug("compile %s failed.\n",cfg->rt_lex_fn);
			goto end;
		}
	}
	if(cfg->rrt_lex_fn)
	{
		cfg->net_rrt=wtk_lexc_compile_file2(lex,cfg->rrt_lex_fn);
		if(!cfg->net_rt)
		{
			wtk_debug("compile %s failed.\n",cfg->rrt_lex_fn);
			goto end;
		}
	}
	if(cfg->parser_lex_fn)
	{
		cfg->net_parser=wtk_lexc_compile_file2(lex,cfg->parser_lex_fn);
		if(!cfg->net_parser)
		{
			wtk_debug("compile %s failed.\n",cfg->parser_lex_fn);
			goto end;
		}
	}
	if(cfg->pa_lex_fn)
	{
		cfg->net_pa=wtk_lexc_compile_file2(lex,cfg->pa_lex_fn);
		if(!cfg->net_pa)
		{
			wtk_debug("compile %s failed.\n",cfg->pa_lex_fn);
			goto end;
		}
	}
	ret=wtk_owlkv_cfg_update_lex(&(cfg->owlkv),lex);
	if(ret!=0){goto end;}
	ret=0;
end:
	//wtk_debug("net_rrt=%p/%s\n",cfg->net_rrt,cfg->rrt_lex_fn);
	//exit(0);
	return ret;
}
#endif
