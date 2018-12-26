#include "wtk_semfi_cfg.h" 

int wtk_semfi_cfg_init(wtk_semfi_cfg_t *cfg)
{
	cfg->crffn=NULL;
	cfg->lexfn=NULL;
	cfg->net=NULL;
	return 0;
}

int wtk_semfi_cfg_clean(wtk_semfi_cfg_t *cfg)
{
	if(cfg->net)
	{
		wtk_lex_net_delete(cfg->net);
	}
	return 0;
}

int wtk_semfi_cfg_update_local(wtk_semfi_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_str(lc,cfg,lexfn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,crffn,v);
	return 0;
}

int wtk_semfi_cfg_update(wtk_semfi_cfg_t *cfg,wtk_lexc_t *lex)
{
	int ret;

	if(cfg->lexfn)
	{
		cfg->net=wtk_lexc_compile_file2(lex,cfg->lexfn);
		//wtk_debug("lex file %s %d\n", cfg->lexfn, wtk_heap_bytes(cfg->net->heap));
		if(!cfg->net)
		{
			wtk_debug("compile %s failed.\n",cfg->lexfn);
			ret=-1;
			goto end;
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_semfi_cfg_update2(wtk_semfi_cfg_t *cfg,wtk_source_loader_t *sl,wtk_lexc_t *lex)
{
	int ret;

	{
		wtk_rbin2_t *rb=(wtk_rbin2_t*)sl->hook;
		lex->rbin=rb;
	}
	if(cfg->lexfn)
	{
		//wtk_debug("compile %s\n",cfg->lexfn);
		cfg->net=wtk_lexc_compile_file2(lex,cfg->lexfn);
		if(!cfg->net)
		{
			wtk_debug("compile %s failed.\n",cfg->lexfn);
			ret=-1;
			goto end;
		}
	}
	ret=0;
end:
	return ret;
}
