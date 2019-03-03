#include "qtk_conf_file.h"
void qtk_conf_file_update_arg_to_cfg(qtk_conf_file_t *c,wtk_arg_t *arg);

#ifdef WIN32
void qtk_conf_file_set_dir(qtk_conf_file_t *cf)
{
    char buf[2048];

    sprintf(buf,"%s\\speechd.log",cf->framework.work_dir.data);
    cf->log=wtk_log_new(buf);
}
#endif

qtk_conf_file_t* qtk_conf_file_new(char *lib_path,char *cfgfn,wtk_arg_t *arg)
{
	qtk_conf_file_t *conf=0;
	wtk_cfg_file_t *cfg;
	char fn[2048];
	char *text=0;
	int len,ret=-1;

	if(cfgfn)
	{
		sprintf(fn,"%s",cfgfn);
	}else
	{
		sprintf(fn,"%s/base.cfg",lib_path);
	}
	if(wtk_file_exist(fn)==0)
	{
		text=file_read_buf(fn,&len);
	}
	if(!text)
	{
		wtk_debug("%s not exist.\n",fn);
		goto end;
	}
	conf=(qtk_conf_file_t*)wtk_calloc(1,sizeof(*conf));
	qtk_fwd_cfg_init(&(conf->framework));
	cfg=conf->cfg=wtk_cfg_file_new(fn);
	conf->heap=cfg->heap;
	conf->lib_path=wtk_heap_dup_string(cfg->heap,lib_path,strlen(lib_path));
	wtk_cfg_file_add_var_ks(cfg,"pwd",conf->lib_path->data,conf->lib_path->len);
	ret=wtk_cfg_file_feed(cfg,text,len);
	if(ret!=0){goto end;}
	if(arg)
	{
		qtk_conf_file_update_arg_to_cfg(conf,arg);
	}
	ret=qtk_fwd_cfg_update_local(&(conf->framework),cfg->main);
	if(ret!=0)
	{
		wtk_debug("update http local configure failed.\n");
		goto end;
	}
	ret=qtk_fwd_cfg_update(&(conf->framework));
	if(ret!=0)
	{
		wtk_debug("update http local dependents configure failed.\n");
		goto end;
	}
    if(arg)
    {
	    ret=qtk_conf_file_update(conf,arg);
	    if(ret!=0){goto end;}
    }
#ifdef WIN32
    qtk_conf_file_set_dir(conf);
#else
	conf->log=wtk_log_new(conf->framework.log_fn.data);//"./log/http.log");
#endif
	ret=conf->log?0:-1;
end:
	if(ret!=0 && conf)
	{
		qtk_conf_file_delete(conf);
		conf=0;
	}
	if(text)
	{
		free(text);
	}
	return conf;
}

int qtk_conf_file_delete(qtk_conf_file_t *c)
{
    qtk_fwd_cfg_clean(&(c->framework));
	if(c->log)
	{
		wtk_log_delete(c->log);
	}
	if(c->cfg)
	{
		wtk_cfg_file_delete(c->cfg);
	}
	wtk_free(c);
	return 0;
}

void qtk_conf_file_print(qtk_conf_file_t *c)
{
	qtk_fwd_cfg_print(&(c->framework));
	return;
}

void qtk_conf_file_update_arg_to_cfg(qtk_conf_file_t *c,wtk_arg_t *arg)
{
	printf("############## update arg to cfg ##############\n");
	wtk_local_cfg_update_arg(c->cfg->main,arg,1);
	//wtk_str_hash_walk(arg->hash,(wtk_walk_handler_t)qtk_conf_file_update_arg_hash_node,c);
	printf("###############################################\n");
}

int qtk_conf_file_update(qtk_conf_file_t *c,wtk_arg_t *arg)
{
	qtk_fwd_cfg_t *cfg=&(c->framework);
	int port,ret;
	char *p;

	ret=wtk_arg_get_int_s(arg,"p",&port);
	if(ret==0)
	{
		cfg->entry.port=port;
	}
	qtk_http_cfg_update_depend(cfg);
	ret=wtk_arg_get_str_s(arg,"d",&p);
	if(ret==0)
	{
		if(strcmp(p,"true")==0)
		{
			cfg->daemon=1;
		}else if(strcmp(p,"false")==0)
		{
			cfg->daemon=0;
		}
	}
	return 0;
}
