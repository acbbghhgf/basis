#include "wtk_loc_cfg.h"

int wtk_loc_cfg_init(wtk_loc_cfg_t *cfg)
{
	wtk_cd_cfg_init(&(cfg->cd));
	wtk_lfs_cfg_init(&(cfg->lfs));
#ifdef WIN32
#else
	wtk_redirect_cfg_init(&(cfg->redirect));
#endif
#ifdef USE_STATICS
	wtk_statics_cfg_init(&(cfg->statics));
#endif
	wtk_string_set_s(&(cfg->url_root),"/");
	cfg->use_lfs=1;
	cfg->use_root=1;
	cfg->use_statics=1;
	cfg->hash_nslot=13;
	cfg->show_root=1;
	return 0;
}

int wtk_loc_cfg_clean(wtk_loc_cfg_t *cfg)
{
	wtk_cd_cfg_clean(&(cfg->cd));
	wtk_lfs_cfg_clean(&(cfg->lfs));
#ifdef WIN32
#else
	wtk_redirect_cfg_clean(&(cfg->redirect));
#endif
#ifdef USE_STATICS
	wtk_statics_cfg_clean(&(cfg->statics));
#endif
	return 0;
}

int wtk_loc_cfg_update_local(wtk_loc_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc=main;
	wtk_string_t *v;
	int ret;

	wtk_local_cfg_update_cfg_b(lc,cfg,use_lfs,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_root,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_statics,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,hash_nslot,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,url_root,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,show_root,v);
	lc=wtk_local_cfg_find_lc_s(main,"lfs");
	if(lc)
	{
		ret=wtk_lfs_cfg_update_local(&(cfg->lfs),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"cd");
	if(lc)
	{
		ret=wtk_cd_cfg_update_local(&(cfg->cd),lc);
		if(ret!=0){goto end;}
	}
#ifdef WIN32
#else
	lc=wtk_local_cfg_find_lc_s(main,"redirect");
	if(lc)
	{
		ret=wtk_redirect_cfg_update_local(&(cfg->redirect),lc);
		if(ret!=0){goto end;}
	}
#endif
#ifdef USE_STATICS
	lc=wtk_local_cfg_find_lc_s(main,"statics");
	if(lc)
	{
		ret=wtk_statics_cfg_update_local(&(cfg->statics),lc);
		if(ret!=0){goto end;}
	}
#endif
	ret=0;
end:
	return ret;
}

int wtk_loc_cfg_update(wtk_loc_cfg_t *cfg)
{
	int ret;

#ifdef USE_STATICS
	ret=wtk_statics_cfg_update(&(cfg->statics));
	if(ret!=0){goto end;}
#endif
	if(cfg->use_lfs)
	{
		ret=wtk_lfs_cfg_update(&(cfg->lfs));
		if(ret!=0){goto end;}
	}
	ret=wtk_cd_cfg_update(&(cfg->cd));
	if(ret!=0){goto end;}
#ifdef WIN32
#else
	ret=wtk_redirect_cfg_update(&(cfg->redirect));
	if(ret!=0){goto end;}
#endif
end:
	return ret;
}

