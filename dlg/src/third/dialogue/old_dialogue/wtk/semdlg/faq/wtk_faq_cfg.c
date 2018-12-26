#include "wtk_faq_cfg.h" 

int wtk_faq_cfg_init(wtk_faq_cfg_t *cfg)
{
	wtk_tfidf_cfg_init(&(cfg->tifidf));
	wtk_vecfaq_cfg_init(&(cfg->vecfaq));
	wtk_vecdb_cfg_init(&(cfg->vecdb));
	cfg->use_tfidf=0;
	cfg->use_vecdb=1;
	return 0;
}

int wtk_faq_cfg_clean(wtk_faq_cfg_t *cfg)
{
	wtk_tfidf_cfg_clean(&(cfg->tifidf));
	wtk_vecfaq_cfg_clean(&(cfg->vecfaq));
	wtk_vecdb_cfg_clean(&(cfg->vecdb));
	return 0;
}

int wtk_faq_cfg_update_local(wtk_faq_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret;

	lc=main;
	wtk_local_cfg_update_cfg_b(lc,cfg,use_tfidf,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_vecdb,v);
	lc=wtk_local_cfg_find_lc_s(main,"tfidf");
	if(lc)
	{
		ret=wtk_tfidf_cfg_update_local(&(cfg->tifidf),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"vecfaq");
	if(lc)
	{
		ret=wtk_vecfaq_cfg_update_local(&(cfg->vecfaq),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"vecdb");
	if(lc)
	{
		ret=wtk_vecdb_cfg_update_local(&(cfg->vecdb),lc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

int wtk_faq_cfg_update(wtk_faq_cfg_t *cfg)
{
	int ret;

	if(cfg->use_tfidf)
	{
		ret=wtk_tfidf_cfg_update(&(cfg->tifidf));
	}else
	{
		ret=wtk_vecfaq_cfg_update(&(cfg->vecfaq));
	}
	if(ret!=0){goto end;}
	if(cfg->use_vecdb)
	{
		ret=wtk_vecdb_cfg_update(&(cfg->vecdb));
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}


int wtk_faq_cfg_update2(wtk_faq_cfg_t *cfg,wtk_source_loader_t *sl)
{
	int ret;

	if(cfg->use_tfidf)
	{
		return -1;
	}else
	{
		ret=wtk_vecfaq_cfg_update2(&(cfg->vecfaq),sl);
	}
	if(ret!=0){goto end;}
	ret=wtk_vecdb_cfg_update2(&(cfg->vecdb),sl);
	if(ret!=0){goto end;}
	ret=0;
end:
	return ret;
}
