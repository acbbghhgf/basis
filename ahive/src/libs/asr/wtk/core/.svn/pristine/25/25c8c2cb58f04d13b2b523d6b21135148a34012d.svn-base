#include "wtk_main_cfg.h"

wtk_main_cfg_t *wtk_main_cfg_new(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn)
{
	return wtk_main_cfg_new2(cfg_bytes,init,clean,update_lc,update,fn,1);
}

wtk_main_cfg_t *wtk_main_cfg_new7(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,wtk_string_t *dir,int update_cfg,char *data,int len)
{
	wtk_main_cfg_t *cfg;
	void *mc;
	int ret;

	cfg=(wtk_main_cfg_t*)wtk_calloc(1,sizeof(*cfg));
	cfg->init=init;
	cfg->clean=clean;
	cfg->update_lc=update_lc;
	cfg->update=update;
	cfg->update2=0;
	//cfg->update_arg=0;
	cfg->cfg_bytes=cfg_bytes;
	mc=cfg->cfg=wtk_calloc(1,cfg_bytes);
	ret=cfg->init(mc);
	if(ret!=0)
	{
		wtk_debug("init failed.\n");
		goto end;
	}
	if(data)
	{
		cfg->cfile=wtk_cfg_file_new_fn3(dir,data,len,1);
		if(!cfg->cfile)
		{
			ret=-1;goto end;
		}
	}else
	{
		cfg->cfile=0;
	}
	if(update_cfg)
	{
		ret=wtk_main_cfg_update_cfg(cfg);
	}
end:
	if(ret!=0)
	{
		wtk_main_cfg_delete(cfg);
		cfg=0;
	}
	return cfg;
}

wtk_main_cfg_t* wtk_main_cfg_new2(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn,int update_cfg)
{
	wtk_main_cfg_t *cfg;
	void *mc;
	int ret;

	cfg=(wtk_main_cfg_t*)wtk_calloc(1,sizeof(*cfg));
	cfg->init=init;
	cfg->clean=clean;
	cfg->update_lc=update_lc;
	cfg->update=update;
	cfg->update2=0;
	//cfg->update_arg=0;
	cfg->cfg_bytes=cfg_bytes;
	mc=cfg->cfg=wtk_calloc(1,cfg_bytes);
	ret=cfg->init(mc);
	if(ret!=0)
	{
		wtk_debug("init failed.\n");
		goto end;
	}
	if(fn)
	{
		cfg->cfile=wtk_cfg_file_new_fn(fn);
		if(!cfg->cfile)
		{
			wtk_debug("%s invalid.\n",fn);
			ret=-1;goto end;
		}
	}else
	{
		cfg->cfile=0;
	}
	if(update_cfg)
	{
		ret=wtk_main_cfg_update_cfg(cfg);
	}
end:
	if(ret!=0)
	{
		wtk_main_cfg_delete(cfg);
		cfg=0;
	}
	return cfg;
}

wtk_main_cfg_t* wtk_main_cfg_new3(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn,int argc,char **argv)
{
	wtk_arg_t *arg;
	wtk_main_cfg_t *cfg;

	arg=wtk_arg_new(argc,argv);
	cfg=wtk_main_cfg_new4(cfg_bytes,init,clean,update_lc,update,fn,arg);
	wtk_arg_delete(arg);
	return cfg;
}

wtk_main_cfg_t* wtk_main_cfg_new4(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn,wtk_arg_t *arg)
{
	return wtk_main_cfg_new5(cfg_bytes,init,clean,update_lc,update,fn,arg,0);
}

wtk_main_cfg_t* wtk_main_cfg_new5(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn,wtk_arg_t *arg,char *cfg_section)
{
	return wtk_main_cfg_new6(cfg_bytes,init,clean,update_lc,update,0,fn,arg,cfg_section);
}

wtk_main_cfg_t* wtk_main_cfg_new6(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,
		wtk_main_cfg_update_arg_f update_arg,
		char *fn,wtk_arg_t *arg,char *cfg_section)
{
	wtk_main_cfg_t *cfg;
	int ret=0;

	cfg=wtk_main_cfg_new2(cfg_bytes,init,clean,update_lc,update,fn,0);
	if(!cfg){goto end;}
	if(arg)
	{
		wtk_local_cfg_update_arg(cfg->cfile->main,arg,1);
	}
	ret=wtk_main_cfg_update_cfg2(cfg,cfg_section);
	if(ret!=0)
	{
		wtk_debug("update cfg failed.\n");
		goto end;
	}
	if(update_arg)
	{
		update_arg(cfg->cfg,arg);
	}
end:
	if(ret!=0)
	{
		wtk_main_cfg_delete(cfg);
		cfg=0;
	}
	return cfg;
}

int wtk_main_cfg_delete(wtk_main_cfg_t *cfg)
{
	if(cfg->cfg)
	{
		cfg->clean(cfg->cfg);
		wtk_free(cfg->cfg);
	}
	if(cfg->cfile)
	{
		wtk_cfg_file_delete(cfg->cfile);
	}
	wtk_free(cfg);
	return 0;
}

int wtk_main_cfg_bytes(wtk_main_cfg_t *cfg)
{
	int bytes=cfg->cfg_bytes;

	if(cfg->cfile)
	{
		bytes+=wtk_cfg_file_bytes(cfg->cfile);
	}
	return bytes;
}

void wtk_main_cfg_update(wtk_main_cfg_t *cfg,int argc,char **argv)
{
	wtk_arg_t *arg;

	arg=wtk_arg_new(argc,argv);
	wtk_local_cfg_update_arg(cfg->cfile->main,arg,0);
	wtk_arg_delete(arg);
}

int wtk_main_cfg_update_cfg_lc(wtk_main_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	int ret;
	void *mc;

	mc=cfg->cfg;
	ret=cfg->update_lc(mc,lc);
	if(ret!=0)
	{
		wtk_local_cfg_print(lc);
		wtk_debug("update lc failed\n");
		goto end;
	}
	ret=cfg->update(mc);
	if(ret!=0)
	{
		wtk_local_cfg_print(lc);
		wtk_debug("update failed\n");
		goto end;
	}
end:
	return ret;
}

int wtk_main_cfg_update_cfg(wtk_main_cfg_t *cfg)
{
	if(cfg->cfile)
	{
		return wtk_main_cfg_update_cfg_lc(cfg,cfg->cfile->main);
	}else
	{
		return 0;
	}
}

int wtk_main_cfg_update_cfg2(wtk_main_cfg_t *cfg,char *section)
{
	wtk_local_cfg_t *lc;

	if(section)
	{
		lc=wtk_local_cfg_find_section_lc(cfg->cfile->main,section,strlen(section));
	}else
	{
		lc=cfg->cfile->main;
	}
	return wtk_main_cfg_update_cfg_lc(cfg,lc);
}



wtk_main_cfg_t *wtk_main_cfg_new_str(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *data,int bytes,char *dn)
{
	wtk_main_cfg_t *cfg;
	void *mc;
	int ret;

	cfg=(wtk_main_cfg_t*)wtk_calloc(1,sizeof(*cfg));
	cfg->init=init;
	cfg->clean=clean;
	cfg->update_lc=update_lc;
	cfg->update=update;
	cfg->update2=0;
	//cfg->update_arg=0;
	cfg->cfg_bytes=cfg_bytes;
	mc=cfg->cfg=wtk_calloc(1,cfg_bytes);
	ret=cfg->init(mc);
	if(ret!=0)
	{
		wtk_debug("init failed.\n");
		goto end;
	}
	cfg->cfile=wtk_cfg_file_new();
	//wtk_cfg_file_add_var_ks(cfg->cfile,"pwd","res",3);
	wtk_cfg_file_add_var_ks(cfg->cfile,"pwd",dn,strlen(dn));
	ret=wtk_cfg_file_feed(cfg->cfile,data,bytes);
	if(ret!=0)
	{
		goto end;
	}
	//wtk_local_cfg_print(cfg->cfile->main);
	if(update_lc)
	{
		ret=update_lc(cfg->cfg,cfg->cfile->main);
		if(ret!=0){goto end;}
	}
	if(update)
	{
		ret=update(cfg->cfg);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	if(ret!=0)
	{
		wtk_main_cfg_delete(cfg);
		cfg=0;
	}
	return cfg;
}
